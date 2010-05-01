#
#while ( <> )
#	{
#	chop;						# Removes LF
#
#	s/\s*$//;					# Strip trailing whitespace
#
#	if ( $_ )
#		{
#		@list = split( /\"/ );
#		foreach $line (@list)
#			{
#			$line =~ s/[^A-Za-z_0-9{};\[\]]//g;
#			print $line, " ";
#			}
#		print "\n";
#		}
#	}
#
#exit

while ( <> )
	{
	chop;						# Remove LF
	s/\s*$//;					# Strip trailing whitespace
	if ( $_ )
		{
		@list = split( /\"/ );
        for($count=1;$count <= @list; $count+=2)
        {
            $list[$count] =~ s/[^A-Za-z_0-9]//g;
        }
		$list[$count] =~ s/[^A-Za-z_0-9]//g;
		print @list, "\n";
		}
	}
