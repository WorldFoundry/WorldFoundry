## script to build entire contents of CD data

$nLevel = 0;

$sector_align_chunk = "{ 'ALGN' .align( 2048 ) }\n";

print "{ 'GAME'\n";
print "\t{ 'TOC'\n";
print "\t\t'SHEL'\t.offsetof( ::'GAME'::'SHEL' )\t.sizeof( ::'GAME'::'SHEL' )\n";

$body .= "\t{ 'SHEL' [ \"shell.tcl\" ]\n";
$body .= "\t}\n";
$body .= "\t$sector_align_chunk\n";
$body .= "\n";

while ( <STDIN> )
{
	chop;
	if ( $_ )
	{
		print "\t\t'L" . $nLevel . "'\t.offsetof( ::'GAME'::'L" . $nLevel . "' )\t.sizeof( ::'GAME'::'L" . $nLevel . "' )\n";

		$body .= "\t{\n\t\t'L" . $nLevel . "'\n\t\t$sector_align_chunk\t\t[ \"$_.iff\" ]\n\t}\n";
		$body .= "\t$sector_align_chunk\n";

		++$nLevel;
	}
}

print "\t}\n";
print "\t$sector_align_chunk\n";

print $body;
print "}\n";
