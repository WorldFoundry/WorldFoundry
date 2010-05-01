#!/usr/bin/perl

#File names
$msgtypes_file = "msgtypes.hp";
$msg_outfile = "msgtypes.id";

$msg_num = 0;

open(MSG_IN, "<$msgtypes_file") || die "ERROR: Can't open $msgtypes_file to load Message Types list\n";
open(MSG_OUT, ">$msg_outfile") || die "ERROR: Can't open $msg_outfile write out info for 3DS.\n";

print MSG_OUT "// This file is generated automatically from $msgtypes_file by $0\n";
print MSG_OUT "// DO NOT MODIFY - xina\n";

#assume that there is one Mesage-type per line, followed by a comma
#and perhaps a comment.  We want to ignore commas and comments.

while (<MSG_IN>)
{
	chop;
	/(\w*),\w*/;
	#$comma = rindex($_, ",");
	#$msgtype_name = substr($_, 0, $comma);
	$msgtype_name = $1;
	if (msgtype ne "")
		{
			print MSG_OUT "$msgtype_name = $msg_num\n";
		 	$msg_num = $msg_num + 1;
		}
}

close(MSG_IN)  || die "ERROR: Can't close $msgtypes_file\n";
close(MSG_OUT) || die "ERROR: Can't close $msg_outfile\n";
