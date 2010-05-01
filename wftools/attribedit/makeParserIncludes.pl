#!/usr/bin/perl -w

# format of input file:
#
# NodeName  Semantic_value_type Semantic_value_return_type
#
# e.g.
#
# Struct NONE
# Name char[256] char*
#
# each line in the input file is a terminal or nonterminal in the grammar.
# this script generates C++ function prototypes for recursive descent parsing
# of each terminal and nonterminal. it also declares the subclasses to
# be stored in the parse tree, along with their semantic value field.
#

use strict;
use FileHandle;

my @lines;

@lines = <>;
my $line;

my $fh = FileHandle->new();

# make C++ function prototypes
$fh->open(">parsefuncdefs.hpi");
for $line ( @lines ) {
    $line =~ chomp $line;
    my @parts = split / /,$line; # ignore semantic type
    print $fh "ParseTreeNode* _parse",$parts[0],"(IFFChunkIter *chunkIter);\n";
}
$fh->close();


# make parse tree class definitions (which must declare the semantic type)
$fh->open(">parsenodedefs.hpi");
for $line ( @lines ) {
    $line =~ chomp $line;
    my @parts = split / /,$line;
    my $nodeName = $parts[0];
    my $semanticType;
    my $semanticTypeCount;
    my $semanticReturnType;
    if($#parts > 0) {
	$semanticType = $parts[1];
	my @stParts = split /\[/,$semanticType;
	if ($#stParts > 0) {
	    $semanticType = $stParts[0];
	    $semanticTypeCount = join("","[",$stParts[1]);
	} else {
	    $semanticTypeCount = "";
	}
	$semanticReturnType = $parts[2];
	if(!defined($semanticReturnType)) {
	    $semanticReturnType = $semanticType;
	}
    }
    print $fh "class ",$nodeName,"Node : public ParseTreeNode\n";
    print $fh "{\n";
    if(defined($semanticType)) {
	print $fh " public:\n";
	print $fh "  ", $semanticReturnType, " value() { return _value; }\n";
	print $fh " private:\n";
	print $fh "  ",$semanticType," _value ", $semanticTypeCount, ";\n";
    }
    print $fh "};\n";
}
$fh->close();




