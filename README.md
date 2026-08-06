# CDK Drive Macrokeyboard

## Keyboard Mapping
- Key 1: Read Line
> Type in "R #" where # is line number and it will read line
- Key 2: Read Additional Line
> Reads another line. If you needed to read multiple lines for each subsequent line you would hit this
- Key 3: Finish Line
> Finishes the line. Used for instance on 00TBD or MMI lines where they are never finished. Then you can read that line right after with Key 1 or 2.
- Key 4: Print a TCM RO
> Optimization of key 5. It prints it and automatically changes the payment method to TCM. Can only be used when the RO is purely a TCM. If is has other customer pay lines, you need to use Key 5 and manually put in what is TCM and what is other methods of payment. It will ask for output miles. Just hit the same key if again if they are already correct. It assumes using printer 2 and assumes input miles are already correct. 
- Key 5: Final Close RO
> When RO is already printed. You just need to close at end of day. Starting at PFC screen inside RO. First press starts final close and brings you into cmp. If needed change it. Once that is done hit key 5 again. This will bring you to mileage out. Change it if you need to. Then hit key 5 a third time and it will close the RO and not print it.
- Key 6: Print RO
> Prints the RO. It will ask for the ouput miles. If they are already correct just hit the same key again. If they are not type in the correct miles and hit the same key again. Prints to printer 2. 
- Key 7: Change to Tyler
> If you have an RO where it is at a loss (for instance a free oil change with nothing else profitable on the RO to offset this) you don't want to report in your name. This changes the service advisor to Tyler Cotter so you don't lose money off your gross. Start in PFC screen and type in RO number. It assumes it has already been preinvoiced. It opens then closes it. Then switches to SWR. It will ask you to type in the RO number again. Hit the same key after, it will change to Tyler then switch back the screen to PFC.
- Key 8-12: Not Programmed
> Key 8 through 12 are not programmed at this time. 

## Keyboard Numbering Sequence
Note: Keys are numbered 1-12 not 0-11 as in standard programming convention to line up with standard convention most people use for numbering. 
