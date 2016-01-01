# iokit-dumper
OS X tool for dumping and reconstructing the _IOKit_ classes hierarchy. `iokit-dumper` directly generates DOT files ([see here](https://en.wikipedia.org/wiki/DOT_(graph_description_language)), which can then be processed with `dot` tool.

Keep in mind this tool is in its early release, so stuff may happen. Also, careful when playing with the code, since a wrong read in the kernel will cause a kernel panic.
<br>
__Remember to always slide kernel addresses before reading from them.__

## How to use
You need to have `dot` installed. If not, do:
```
brew install graphviz
```
Then test with:
```
dot -v
```
Now you can generate DOT files with `iokit-dumper`!

Firstly, disable SIP if your system has it present/enabled. If you are on 10.11.1, check out my SIP bypass via a kernel-exploit [here](http://www.github.com/jndok/stfusip).
<br>
Otherwise, just reboot into Recovery Mode and run `csrutil disable` from the Terminal.
<br>
Once SIP is disabled, do:
```
sudo ./iokit-dumper 'kernel'/[kext_bundle_id] [output_file] [overwrite]
```
Note: the `'kernel'` part means that if you want to dump the kernel hierarchy, you should provide the _kernel_ string (without quotes) as the first argument to the tool.
<br>
The `[kext_bundle_id]` is any KEXT bundle ID (obtainable via `kextstat` command on OS X).

The `[output_file]` is obviously the output file path.

The `[overwrite]` parameter is a boolean value (pass `0` or `1`). It specifies whether to override the output file's contents or just append to it.

So, sample usage:
```
./iokit-dumper com.apple.driver.AppleHDA /Users/$USER/Desktop/test.dot
```
Then do:
```
dot -Tpdf test.dot -o test.pdf
```
To process the `test.dot` file and generate a visual graph in PDF format.

#### Readability
DOT graphs are a bit hard on the eye, and tend to take a lot of space horizontally. To improve visual layout, add these properties at the start of the DOT file (just after the `Digraph` declaration, and before the hierarchy):
```
nodesep=[int];  // amount of space between nodes
ranksep=[int];  // amount of space between ranks (increses vertical space)
```
You can just learn to use DOT and add pretty much everything you want to your graph. Future updates will support more graph customization.

## How does it work?
`iokit-dumper` reconstructs the whole _IOKit_ hierarchy of the kernel or a KEXT. This is possible via special objects stored in kernel memory, known as `gMetaClass`es. These objects are instances of the `OSMetaClass` class, which basically provides informations about another class.
<br>
These objects contain interesting informations, such as the class name, the class size, a pointer to the parent class' `gMetaClass`, etc..
<br>
Once we find these objects in memory, we can climb up the hierarchy and reconstruct it.

#### What about iOS?

The code is iOS-friendly, not meaning it works as-it-is on iOS, but that the algorithm can be reapplied in iOS. This is possible since `iokit-dumper` does not rely on symbols (which could be used on OS X to hasten the process), which are not present in iOS (the prelinked-kernel is stripped).

The steps to make it work on iOS would roughly be:
  - Finding a way to read from kernel memory. (On OS X we can read kernel memory directly from the kernel mach port, obtained via `processor_set_tasks` workaround, or via `/dev/kmem`. These methods are obviously unavailable in iOS)
  - Obtaining the kernel slide. We cannot read from kernel addresses without first sliding the addresses. (On OS X we call `kas_info` as root to get the kernel slide)
  - Changing the parsing part. Look for KEXTs in the `__PRELINK_TEXT` segment of the kernel. (iOS stores all KEXTs inside the prelinked-kernel, hence they are not floating around in the file system)
