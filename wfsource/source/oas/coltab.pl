#=============================================================================
# coltab.pl: create collision table from objects.mac and objects.ctb
#=============================================================================

$collisionDefault = '((CI_NOTHING<<16)|CI_NOTHING)';
$enumfile = 'objects.e';		# object enumeration
$tabfile = 'objects.ctb';		# input object exception table
$outfile = 'objects.car';

#=============================================================================

# Open the enumeration file
open(INFO, $enumfile);

@objectEnum;
while ($line = <INFO>)
{
#	@objectEnum += $line;
	chop $line;												# kill cr/lf
	if($line =~ /[^ \t]/)
	 {
		if(!(($line =~ /\/\*/) || $line =~ /#/))
	 	{
			substr($line,-6) = '';							# kill trailing _KIND,
			$line =~ s/[ \t]+//g;
			push(@objectEnum, $line);
	 	}
	}

}
close(INFO);                    # Close the file
$objectCount = @objectEnum;		# get # of objects

# print enumeration
#$lineNo = 0;
#while($lineNo < $objectCount)
#{
#	print "$lineNo $objectEnum[$lineNo]\n";
#	$lineNo++;
#}

# create the array
@collisionArray = ();
for($RowNo=0;$RowNo < $objectCount; $RowNo++)
{
	for($ColNo=0;$ColNo < $objectCount; $ColNo++)
	 {
		push(@collisionArray,$collisionDefault);
	 }
}

# read in objects.ctb
open(TABFILE,$tabfile);
@patchArray;
while ($line = <TABFILE>)
{
	chop $line;												# kill cr/lf
	if($line =~ /[^ \t]/)
	 {
		if(!(($line =~ /\/\//) || $line =~ /#/))
	 	{
			$line =~ s/[ \t]+//g;
			push(@patchArray, $line);
	 	}
	}
}
close(TABFILE);

# print patch list
#$lineNo = 0;
#while($lineNo < @patchArray)
#{
#	print "$lineNo $patchArray[$lineNo]\n";
#	$lineNo++;
#}

# modify array based on objects.ctb
$lineNo = 0;
while($lineNo < @patchArray)
{
	($obj1,$obj2,$objMsg1,$objMsg2) = split(/,/,$patchArray[$lineNo]);
	#print "#$obj1#$obj2#$objMsg1#$objMsg2\n";

	# look up index of obj1 in table
	for($obj1EnumIndex=0;$obj1 ne $objectEnum[$obj1EnumIndex] && $obj1EnumIndex < @objectEnum;$obj1EnumIndex++)
	 {
		#print "checking index $obj1EnumIndex containing $objectEnum[$obj1EnumIndex] against $obj1\n";
	 }
	if($obj1EnumIndex >= @objectEnum)
	 {
		print "Object $obj1 not found!\n";
	 }
	#print "obj1EnumIndex for $obj1 is $obj1EnumIndex\n";

	# look up index of obj2 in table
	for($obj2EnumIndex=0;$obj2 ne $objectEnum[$obj2EnumIndex] && $obj2EnumIndex < @objectEnum;$obj2EnumIndex++)
	 {
		#print "checking index $obj2EnumIndex containing $objectEnum[$obj2EnumIndex] against $obj2\n";
	 }
	if($obj2EnumIndex >= @objectEnum)
	 {
		print "Object $obj2 not found!\n";
	 }
	#print "obj2EnumIndex for $obj2 is $obj2EnumIndex\n";

	# ok, now actually patch table
	$tableIndex = $obj1EnumIndex + ($obj2EnumIndex*$objectCount);
	#print "TableIndex  = $tableIndex\n";

	# error checking to prevent multiple assignments
	if(@collisionArray[$tableIndex] ne $collisionDefault)
	 {
		print "Error!: association $obj1:$obj2 has already been defined!\n";
	 }

	@collisionArray[$tableIndex] = "(($objMsg1<<16)|$objMsg2)";
	$tableIndex = $obj2EnumIndex + ($obj1EnumIndex*$objectCount);
	#print "TableIndex2  = $tableIndex\n";
	@collisionArray[$tableIndex] = "(($objMsg2<<16)|$objMsg1)";

	$lineNo++;
}

#print the output file
open(OUTFILE, ">$outfile");              # Open the enumeration file

print OUTFILE "//============================================================================\n";
print OUTFILE "// objects.car: collision array table, included by room.cc\n";
print OUTFILE "// created by coltab.pl from objects.e and objects.ctb, DO NOT MODIFY\n";
print OUTFILE "//============================================================================\n";
print OUTFILE "//                0                             1                             2                             3                             4                             5                             6                             7                             8                             9                             10                            11                            12                            13                            14                            15                            16                            17                             18                           19                            20                            21                            22                            23                            24                            25                            26                            27                            28                            29                            30                            31                            32                            33                            34                            35";
print OUTFILE "\ncollisionInteraction collisionInteractionTable[PhysicalObject::MAX_OBJECT_TYPES][PhysicalObject::MAX_OBJECT_TYPES] =\n{\n";

# print the array
for($RowNo=0;$RowNo < $objectCount; $RowNo++)
{
	@tempArray = @collisionArray[$RowNo];
	print OUTFILE "{";
	for($ColNo=0;$ColNo < $objectCount; $ColNo++)
	 {
		print OUTFILE "@collisionArray[$ColNo+($RowNo*$objectCount)]";
		if($ColNo != $objectCount-1)
		 {
			print OUTFILE ",";
		 }
	 }
	print OUTFILE "}";
	if($RowNo != $objectCount-1)
	 {
		print OUTFILE ",";
	 }
	print OUTFILE "\n";
	@collisionArray[RowNo] = @tempArray;
}
print OUTFILE "};\n";

#=============================================================================
