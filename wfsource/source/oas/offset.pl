#!/usr/bin/perl

$offset = 0;
while ( <> )
	{
	if ( /define[ \t]+(.+)[ \t]+(\d+)[ \t]*\)[ \t]*;(.*)/ )
		{
		$_ = "\t( define $1 $offset )\t;$3\n";
		$offset += $2;
		}
	print;
	}
