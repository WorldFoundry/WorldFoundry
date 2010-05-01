#!/usr/bin/perl -w

use DirHandle;

$bVerbose = 1;

&processDir( ".", makeUnix );

sub makeUnix
{
    my ( $path, $filename ) = @_;
    my $fullname = $path . $filename;
    $_ = $filename;

    # lowercase everything
    tr/A-Z/a-z/;
    # fix "Makefile"
    s/^makefile/Makefile/;
    # and these should be all uppercase
    s/^changelog/CHANGELOG/;
    s/^todo/TODO/;
    s/^cvs$/CVS/;       # this one too, I think


    # Check for 8.3 ~ fix and issue warning
    print "WARNING: 8.3: $_\n" if ( /~/ );

    print "$fullname => $path$_" if ( $bVerbose );
    print "\t$!" if ( !rename $fullname, "$path$_" ) && $bVerbose;
    print "\n" if ( $bVerbose );
}

sub processDir
{
    my ( $dir, $fn ) = @_;

    # Allow the directory to be processed
    &$fn( "./", $dir );

    my $d = new DirHandle $dir;
    if ( defined $d )
    {
        while ( defined( $_ = $d->read ) )
        {
            next if ( $_ eq "." );
            next if ( $_ eq ".." );
            ( -d $_ ) ? processDir( "$dir/$_", $fn ) : &$fn( "$dir/", $_ );
        }
    }
}

