#!/usr/bin/perl

#default file
$lidfile = "unit4.lid";

# if .lid is specified as argument, use that lid file.
if ($#ARGV >= 0) {
    $lidfile = $ARGV[0];
}

$nslabs = `lidquery $lidfile SLAB_COUNT`;

for ($slabn = 0; $slabn < $nslabs; $slabn++) {
    # These all automatically set 
    $basename = `lidquery $lidfile $slabn SLAB_BASENAME\n`;
    chop $basename;
    $nu = `lidquery $lidfile $slabn SAMPLES_U\n`;
    $nv = `lidquery $lidfile $slabn SAMPLES_V\n`;

    print "Fixing Slab #$slabn ($basename)\n";


    for ($v=0; $v < $nv; $v++) {
	for ($u=0; $u < $nu; $u++) {
	    # Figure out file names
	    $oldname = "$basename.$u.$v.tif";
	    $newname = "$basename.$u.$v.rgb";

	    # Do commands
	    if (-e $oldname) {
		if (-e $newname) {
		    # don't convert if it's already been done
		} else {
		    # old one exists, new one doesnt -- convert!
		    print("fromtiff $oldname $newname\n");
		    system("fromtiff $oldname $newname\n");
		}
	    }
	}
    }
}
