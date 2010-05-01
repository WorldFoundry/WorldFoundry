#!/usr/bin/perl -w

$last="";                 
$input = <STDIN>;
while($input && $input ne "")
{
    $input =~ s/[ \tr]+\n$/\n/;
    if($input ne "\n")
    {
        print $input;
    }
    else
    {
        if($last ne $input)
        {
            print $input;               # so that single newlines get through
        }
    }
    $last=$input;   
    $input = <STDIN>;
}



