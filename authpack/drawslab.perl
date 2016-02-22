#!/usr/common/bin/perl

# Usage:  drawslab.perl [<slabno> <u> <v> [<lidfile>]]
# 
#       - If no arguments are specified, it draws all slabs listed
#         in the lid file.
#       - This program requires the lidquery executable
#

# This is the default lid file, if it is not specified on the
# command line
$LIDFILE = "unit4.lid";

# Set this variable to 1 to get proper synthetic aperture.
# Note that this slows down rendering, especially for wide
# synthetic apertures (i.e. low numbers of u and v).
$DOAPERTURE = 0;

# Define a few constants
$X=0; $Y=1; $Z=2; $W=3; $S=4; $T=5;
$EPSILON = 0.000001;

# Parse the <slabno> <u> <v> arguments
if ($#ARGV >= 2) {
    $DOSINGLEIMAGE = 1;
    $singleslabno = int($ARGV[0]);
    $singleuno = int($ARGV[1]);
    $singlevno = int($ARGV[2]);
    print "Drawing single image: slab: $singleslabno u: $singleuno ".
	"v: $singlevno\n";
}

# Parse the <lidfile> argument
if ($#ARGV >= 3) {
    $LIDFILE = $ARGV[3];
}
print "Drawing from $LIDFILE ...\n";

$nslabs = `lidquery $LIDFILE SLAB_COUNT`;

for ($slabn = 0; $slabn < $nslabs; $slabn++) {
    # Draw the slab, unless we're only doing a single slab, and it isn't
    # this one.
    if (!$DOSINGLEIMAGE || $slabn == $singleslabno) {
        # get slab dimensions
        chop ($basename = `lidquery $LIDFILE $slabn SLAB_BASENAME`);
        $nu = int(`lidquery $LIDFILE $slabn SAMPLES_U`);
        $nv = int(`lidquery $LIDFILE $slabn SAMPLES_V`);
        $ns = int(`lidquery $LIDFILE $slabn SAMPLES_S`);
        $nt = int(`lidquery $LIDFILE $slabn SAMPLES_T`);

        # Format
        $format = `lidquery $LIDFILE $slabn FORMAT`;

        # Read uv geometry
        for ($i=0; $i < 24; $i ++) {
            $vertex = int($i/6);
            $component = int($i % 6);
            $uvgeom[$i] = 0.0 +
            `lidquery $LIDFILE $slabn SLAB_UV_V_C $vertex $component`;
        }

        # Read st geometry
        for ($i=0; $i < 24; $i ++) {
            $vertex = int($i/6);
            $component = int($i % 6);
            $stgeom[$i] = 0.0 + 
            `lidquery $LIDFILE $slabn SLAB_ST_V_C $vertex $component`;
        }

        # Flip the z components, because RenderMan uses a left-handed
        # coordinate system
        for ($i=$Z; $i < 24; $i += 6) {
            $uvgeom[$i] = 0.0 - $uvgeom[$i];
            $stgeom[$i] = 0.0 - $stgeom[$i];
        }
        
        # ============================================================
        # This handles a small subset of possible slabs, in order
        # to keep the calculations simple.  It assumes that both the
        # uv and st planes are parallel to each other, and parallel
        # to the Y axis.  Furthermore, it expects that the Y axis
        # is the up vector.
        #     If you want to do arbitrary slabs, then you can
        # add code to transform the slabs (and the object) into the
        # correct orientation.  
        #
        # A few other caveats:
        #    - It only checks the W component of one vertex of each
        #      plane.  It assumes that all vertices have the same
        #      W value, and that the value is always either 1 or 0.
        #    - If a plane is at infinity, it actually divides by
        #      $EPSILON instead of zero.  This makes it easier to set
        #      up the proper field of view.
        # ============================================================

        # Calculate the rotation angle around the y axis.
        $opposite = $stgeom[6+$Z] - $stgeom[$Z];
        $adjacent = $stgeom[6+$X] - $stgeom[$X];
        $rotanglerad = atan2($opposite, $adjacent);
        $rotangledeg = $rotanglerad * 180.0 / 3.1415926536;

        # Rotate the slabs around the y axis 
        for ($i=0; $i < 24; $i += 6) {
            #uv
            $newx = cos(-$rotanglerad)*$uvgeom[$i+$X] - 
            sin(-$rotanglerad)*$uvgeom[$i+$Z];
            $newz = cos(-$rotanglerad)*$uvgeom[$i+$Z] + 
            sin(-$rotanglerad)*$uvgeom[$i+$X];
            $uvgeom[$i + $X] = $newx;
            $uvgeom[$i + $Z] = $newz;
            #st
            $newx = cos(-$rotanglerad)*$stgeom[$i+$X] - 
            sin(-$rotanglerad)*$stgeom[$i+$Z];
            $newz = cos(-$rotanglerad)*$stgeom[$i+$Z] + 
            sin(-$rotanglerad)*$stgeom[$i+$X];
            $stgeom[$i + $X] = $newx;
            $stgeom[$i + $Z] = $newz;
        }

        # Check the w component of one vertex
        if ($uvgeom[$W] == 0) {
            # uv plane is at infinity -> orthographic projection
            $uvgeom[$X] /= $EPSILON;
            $uvgeom[$Y] /= $EPSILON;
            $uvgeom[$Z] /= $EPSILON;
        } else { 
            if ($stgeom[$W] == 0) {
            # st plane is at infinity -> fixed field of view
            $stgeom[$X] /= $EPSILON;
            $stgeom[$Y] /= $EPSILON;
            $stgeom[$Z] /= $EPSILON;
            } 
        }

        for ($u = 0; $u < $nu; $u++) {
            for ($v = 0; $v < $nv; $v++) {
                # draw the image, unless we're doing a single image
                if (!$DOSINGLEIMAGE || ($u == $singleuno &&
                            $v == $singlevno)) {
                    
                    # Check the w component of one vertex
                    if ($uvgeom[$W] == 0) {
                        if ($stgeom[$W] == 0) {
                            print("Error!  both uv and st planes cannot be\n".
                              "at infinity!");
                        }
                        # uv plane is at infinity -> orthographic projection
                        print "Drawing $u,$v: Doing orthographic projection\n";
                    } else { 
                        if ($stgeom[$W] == 0) {
                            # st plane is at infinity -> fixed field of view
                            print("Drawing $u,$v: Doing fixed-field ".
                              "perspective projection\n");
                        } else {
                            # uv plane is not at infinity, neither is st.
                            print("Drawing $u,$v: Doing (finite) perspective ".
                              "projection\n");
                        }
                    }
                    # This assumes that it has been rotated into the z axis.
                    $fl = $stgeom[0];
                    $fr = $stgeom[6];
                    $fb = $stgeom[1];
                    $ft = $stgeom[13];
                    $fz = $stgeom[2];
                    $nl = $uvgeom[0];
                    $nr = $uvgeom[6];

                    $nb = $uvgeom[1];
                    $nT = $uvgeom[13];
                    $nz = $uvgeom[2];

                    $z = $fz - $nz;
                    $cx = $nl+($nr-$nl)*$u/($nu-1.);
                    $cy = $nb+($nT-$nb)*$v/($nv-1.);
                    
                    # Generate a world.u.v.rib file from world.rib
                    # template.
                    $sedstr = ("sed " . 
                           "-e \'s/\$slabn/$slabn/g\' ".
                           "-e \'s/\$u/$u/g\' ".
                           "-e \'s/\$v/$v/g\' ".
                           "-e \'s+\$basename+$basename+g\' ".
                           " < world.rib > world.$slabn.$u.$v.rib\n");
                    system($sedstr);

                    # Generate a window.u.v.rib file.
                    open(WINDOW, ">window.$slabn.$u.$v.rib");
                    print WINDOW "Projection \"perspective\"\n";
                    print WINDOW "ScreenWindow ". ($fl-$cx)/$z ." ".
                    ($fr-$cx)/$z ." ". ($fb-$cy)/$z ." ". 
                        ($ft-$cy)/$z ."\n";
                    # The translate after the projection actually
                    # becomes a shear.
                    print WINDOW "Translate ". -$cx ." ". -$cy ." ". 
                    -$nz ."\n";

                    # This rotates the *object*, rather than the
                    # uv and st planes.  Thus, $rotangle (set above)
                    # should be the opposite angle of rotation from
                    # the true slab rotation in the final .lid file.
                    print WINDOW "Rotate ".($rotangledeg)." 0 1 0\n";

                    print WINDOW "Display \"$basename.". $u .".". $v 
                    .".tif\" \"file\" \"rgba\"\n";
                    # Set the proper blurring
                    if ($DOAPERTURE) {
                    print WINDOW "DepthOfField ". $nu*$z/($nr-$nl) .
                        " $z $z\n";
                    }
                    print WINDOW "Format $ns $nt 1\n";
                    close(WINDOW);
                    # rmanpp -- a simple rman preprocessor,
                    # included in the package.
                    system("rmanpp world.$slabn.$u.$v.rib | prman -cwd . -progress\n");
                    system("rm window.$slabn.$u.$v.rib");
                    system("rm world.$slabn.$u.$v.rib");
                }
            }
        }
    }
}




