

$X = 0;

while ( <> )
{
	chop;
	( $newdel ) = split( /,/ );

	if ( $newdel eq "NEW" )
	{
		( $newdel, $size, $rounded_size, $file, $line, $addr, $comments ) = split( /,/ );

		if ( $comments ne "ViewPort Primitives" )
		{
			$file .= "/" . $comments . "/" . $line;
			++$nAllocations{ $file };
			$cbSizeTotal{ $file } += $size;
			$cbSizeLevel{ $file } += $size;
			$cbLargestSizeLevel{ $file } = $cbSizeLevel{ $file } if ( $cbSizeLevel{ $file } > $cbLargestSizeLevel{ $file } );

#			$iffread = "iffread.cc";
#			print "iffread.cc: total=$cbSizeTotal{$iffread} level=$cbSizeLevel{$iffread} largest=$cbLargestSizeLevel{$iffread}\n";
			$file[ $X ] = $file;
			$address[ $X ] = $addr;
			$size[ $X ] = $size;
#			print "new entry $X, file[$X]=$file[$X], address[$X]=$address[$X]\n";
			++$X;
		}
	}

	if ( $newdel eq "DEL" )
	{
		( $newdel, $addr, $comments ) = split( /,/ );

		# find address
		for ( $idx = $X-1; $idx >= 0; --$idx )
		{
#			print "looking: $addr, address[ $idx ] = $address[ $idx ]\n";
			if ( $address[ $idx ] eq $addr )
			{
#				print "idx = $idx, found allocation @ $address[ $idx ] in file $file[ $idx ]\n";
				$cbSizeLevel{ $file[ $idx ] } -= $size[ $idx ];
				$idx = -2;		# break outta here
			}
		}
		print "ERROR: Couldn't find address $addr\n" if ( $idx == -1 );
	}
}

print "Total,Largest,Level,File,numAllocations\n";
while ( ($holder, $record) = each( %nAllocations ) )
{
	printf "%9d,%9d,%9d,%s,%9s allocation%s\n", $cbSizeTotal{ $holder },
		$cbLargestSizeLevel{ $holder }, $cbSizeLevel{ $holder }, $holder,
		$record, ($record==1 ? "" : "s");

	$total += $cbSizeTotal{ $holder };
	$largest += $cbLargestSizeLevel{ $holder };
	$level += $cbSizeLevel{ $holder };
	$allocations += $nAllocations{ $holder };
}

#print "=======\t=======\t=======\t===============\t====================\n";
#print $total . "," . $largest . "," . $level . ",Total," . $allocations . " allocation(s)\n";
printf "%9d,%9d,%9d,%s,%9s allocation%s\n", $total, $largest, $level, "TOTAL", $allocations, "s";
