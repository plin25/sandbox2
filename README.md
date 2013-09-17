sandbox2
========

Sandbox -- homework for NYU-Poly's CS 9163 Application Security

After making:

./sandbox prog_file [data_file]

Preloading from Data File has NOT been tested yet :(

SYNTAX
========

Terminology:

-[X] means the value at memory location X.

-$X denotes that a memory address is desired.

-#L denotes that a label is desired. If a label is used in a Branching instruction it must be set somewhere using a Label instruction.

-NUM denotes an integer is desired.

-Memory addresses range from 1 to 1048576. If the address is negative, then the value stored at the positive value of the address is used instead. For example: if [20]=3, then "COP -20 10" will be translated to "COP 3 10"

COMMAND	ARGUMENTS		//	MEANING


// Setting/Arithmetic

ADD		$T	$S1	$S2		//	$T<-$S1+$S2

SUB		$T	$S1	$S2		//	$T<-$S1-$S2

COP		$T	$S			//	$T<-$S

SET		$T	NUM			//	$T<-NUM


// Branching

LABEL	#L				//	define label #L

BEQ		#L	$S1	$S2		//	IF $S1=$S2 => #L

BNE		#L	$S1	$S2		//	IF $S1!=$S2 => #L

BG		#L	$S1	$S2		//	IF $S1>$S2 => #L

BL		#L	$S1	$S2		//	IF $S1<$S2 => #L

BGE		#L	$S1	$S2		//	IF $S1>=$S2 => #L

BLE		#L	$S1	$S2		//	IF $S1<=$S2 => #L


// Miscellaneous

DISP	$T				//	display $T

EXIT	NUM				//	"Return code NUM" if NUM!=0, exit gracefully ohterwise
