
open FILE, "disc.raw" || die "Can't open disc.raw: $!\n";
binmode FILE;
open OUT, ">disc.iso" || die "Can't open output file disc.iso: $!\n";
binmode OUT;

$RAW_SECTOR = 2352;
$SECTOR = 2048;

$HEADER_SIZE = 16;

$idxSector = 0;

while ( $nBytesRead = sysread( FILE, $buf, $RAW_SECTOR ) )
{
	printf( "Warning: file is not a multiple of $RAW_SECTOR bytes\n" ) if ( $nBytesRead != $RAW_SECTOR );
	printf "\rReading sector $idxSector";

	syswrite( OUT, $buf, $SECTOR, $HEADER_SIZE );

	++$idxSector;
}


close OUT;
close FILE;
