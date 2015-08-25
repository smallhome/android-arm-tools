A combination of tools to deal with bad memory on a Linux machine with
root access but limited access to kernel configuration. (The exact use case
is an Android tablet with a locked bootloader but a local root exploit).
It's an ugly hack, but the circumstances were ugly too :-)

mtester:
        A simple memory tester that tries to find the physical address
        of faulty memory. 

        Works like this:
        - Allocate a large memory buffer and mlock() it.
        - Fill the buffer with a repeating random pattern.
        - Go through the buffer again, checking that the random pattern
          repeats correctly.
        - If not, try to find the physical address corresponsing to that
          virtual address space page. Do this by opening /dev/mem, and
          looking for a page there whose contents exactly match the contents
          of the virtual address page with a failure. The expectation is
          that it'll mostly be unique (working pages will have the random
          pattern written flawlessly, and so should not match the broken
          page's apparent contents).

        Run as root with "mtester XXX", where XXX is as much memory
        (in megabytes) as the machine can spare. It'll output the faulty
        pages, and some diagnostic information in case the results don't
        look entirely trustworthy.

        You'll probably want to run the program multiple times in a row,
        to account for various random factors.

        Once that is done, pipe the combined output of these several
        rust to the included merge-ranges.pl. This will merge adjacent
        pages to larger regions, and expand each region to be aligned
        at 128kB boundaries at both ends.

mlocker:
        Prevent a faulty physical memory region from being handed to programs.

        Works like this:
        - mmap the specified regions in /dev/mem
        - exit()
        
        ... and that's it. I expected that the program would at a minimum need
        to mlock() the mapped regions + stay running for ever (and in the worst
        case jump through similar hoops as mtester to find the relevant
        physical addresses in the first place). But in practice none of that
        was needed. Instead it turns out that just the act of mmaping a bit
        of /dev/mem is sufficient to lock that memory out until the next
        reboot.

        I haven't yet read the kernel source to figure out what the mechanism
        behind this would be. If any kernel hacker knows what is going on
        (or wants to argue that this isn't what actually happening), I'd
        love to hear from you :-)
