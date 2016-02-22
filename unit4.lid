# LIF1.0
# Lightfield library, version 1.0
#
# Copy and modify this file to describe a set of images for
# generating a light field.
#
# Any line beginning with '#' is a comment.
#
# The light field is made up of "slabs".  Each slab is
# a regular array of images, taken from regular intervals 
# on a uv plane, looking at an st plane.
#

# ==================== slab 0 specification ====================
#
# The first line gives the slab number (each number must be unique,
# and less than LF_MAXSLABS (currently 30).  It also lists the
# basename for the array of images.  For example, the line
# 	SLAB		0 data/lion
# would tell the program to look in the data directory for files
# of the format "lion.u.v.rgb", i.e.
# lion.0.0.rgb, lion.0.1.rgb, ..., lion.16.16.rgb, etc.

SLAB		0 N4/dragon

# These next four lines describe the location of the uv plane.  
# Each line describes one vertex of the quadrilateral.  The first
# four numbers of each line give the x,y,z,w coordinates of the
# corner vertex, and the last two numbers give texture coordinates,
# describing how to map u and v to the quadrilateral.  
# The four vertices should be coplanar.


SLAB_UV		
		-9 -9 10 1 	0 0
		 9 -9 10 1 	1 0
		 9  9 10 1 	1 1
		-9  9 10 1 	0 1

# These next four lines describe the location of the st plane.  
# Each line describes one vertex of the quadrilateral.  The first
# four numbers of each line give the x,y,z,w coordinates of the
# corner vertex, and the last two numbers give texture coordinates,
# describing how to map u and v to the quadrilateral.
#
# The four vertices should be coplanar.

SLAB_ST		
		-10 -10 5 1 	0 0
		 10 -10 5 1 	1 0
		 10  10 5 1 	1 1
		-10  10 5 1 	0 1


# This line describes the dimensions of the light field array in
# u and v. (i.e. the size of the array of images.)

SAMPLES_UV	4 4

# This line describes the dimensions of the light field array in
# s and t. (i.e. the size of each image (in pixels)).

SAMPLES_ST	128 128

# This line describes the format of the images.

FORMAT		RGB

# -------------------- end of slab  --------------------


# ==================== slab 1 specification ====================
#
# The first line gives the slab number (each number must be unique,
# and less than LF_MAXSLABS (currently 30).  It also lists the
# basename for the array of images.  For example, the line
# 	SLAB		0 data/lion
# would tell the program to look in the data directory for files
# of the format "lion.u.v.rgb", i.e.
# lion.0.0.rgb, lion.0.1.rgb, ..., lion.16.16.rgb, etc.

SLAB		1 W4/dragon

# These next four lines describe the location of the uv plane.  
# Each line describes one vertex of the quadrilateral.  The first
# four numbers of each line give the x,y,z,w coordinates of the
# corner vertex, and the last two numbers give texture coordinates,
# describing how to map u and v to the quadrilateral.  
#
# The four vertices should be coplanar.


SLAB_UV		
                10.000000 -10.000000 10.000000 1.000000 	0 0
                10.000000 -10.000000 -10.000000 1.000000 	1 0
                10.000000 10.000000 -10.000000 1.000000 	1 1
                10.000000 10.000000 10.000000 1.000000 		0 1

# These next four lines describe the location of the st plane.  
# Each line describes one vertex of the quadrilateral.  The first
# four numbers of each line give the x,y,z,w coordinates of the
# corner vertex, and the last two numbers give texture coordinates,
# describing how to map u and v to the quadrilateral.
#
# The four vertices should be coplanar.

SLAB_ST		
                0.000000 -1.100000 1.100000 1.000000 	0 0
                0.000000 -1.100000 -1.100000 1.000000 	1 0
                0.000000 1.100000 -1.100000 1.000000 	1 1
                0.000000 1.100000 1.100000 1.000000 	0 1


# This line describes the dimensions of the light field array in
# u and v. (i.e. the size of the array of images.)

SAMPLES_UV	4 4

# This line describes the dimensions of the light field array in
# s and t. (i.e. the size of each image (in pixels)).

SAMPLES_ST	128 128

# This line describes the format of the images.

FORMAT		RGB

# -------------------- end of slab  --------------------


# ==================== slab 2 specification ====================
#
# The first line gives the slab number (each number must be unique,
# and less than LF_MAXSLABS (currently 30).  It also lists the
# basename for the array of images.  For example, the line
# 	SLAB		0 data/lion
# would tell the program to look in the data directory for files
# of the format "lion.u.v.rgb", i.e.
# lion.0.0.rgb, lion.0.1.rgb, ..., lion.16.16.rgb, etc.

SLAB		2 S4/dragon

# These next four lines describe the location of the uv plane.  
# Each line describes one vertex of the quadrilateral.  The first
# four numbers of each line give the x,y,z,w coordinates of the
# corner vertex, and the last two numbers give texture coordinates,
# describing how to map u and v to the quadrilateral.  
#
# The four vertices should be coplanar.


SLAB_UV		
                10.000000 -10.000000 -10.000000 1.000000 	0 0
                -10.000000 -10.000000 -10.000000 1.000000	1 0
                -10.000000 10.000000 -10.000000 1.000000 	1 1
                10.000000 10.000000 -10.000000 1.000000 	0 1

# These next four lines describe the location of the st plane.  
# Each line describes one vertex of the quadrilateral.  The first
# four numbers of each line give the x,y,z,w coordinates of the
# corner vertex, and the last two numbers give texture coordinates,
# describing how to map u and v to the quadrilateral.
#
# The four vertices should be coplanar.

SLAB_ST		
                1.100000 -1.100000 0.000000 1.000000 	0 0
                -1.100000 -1.100000 0.000000 1.000000 	1 0
                -1.100000 1.100000 0.000000 1.000000 	1 1
                1.100000 1.100000 0.000000 1.000000 	0 1


# This line describes the dimensions of the light field array in
# u and v. (i.e. the size of the array of images.)

SAMPLES_UV	4 4

# This line describes the dimensions of the light field array in
# s and t. (i.e. the size of each image (in pixels)).

SAMPLES_ST	128 128

# This line describes the format of the images.

FORMAT		RGB

# -------------------- end of slab  --------------------


# ==================== slab 3 specification ====================
#
# The first line gives the slab number (each number must be unique,
# and less than LF_MAXSLABS (currently 30).  It also lists the
# basename for the array of images.  For example, the line
# 	SLAB		0 data/lion
# would tell the program to look in the data directory for files
# of the format "lion.u.v.rgb", i.e.
# lion.0.0.rgb, lion.0.1.rgb, ..., lion.16.16.rgb, etc.

SLAB		3 E4/dragon

# These next four lines describe the location of the uv plane.  
# Each line describes one vertex of the quadrilateral.  The first
# four numbers of each line give the x,y,z,w coordinates of the
# corner vertex, and the last two numbers give texture coordinates,
# describing how to map u and v to the quadrilateral.  
#
# The four vertices should be coplanar.


SLAB_UV		
                -10.000000 -10.000000 -10.000000 1.000000	0 0
                -10.000000 -10.000000 10.000000 1.000000 	1 0
                -10.000000 10.000000 10.000000 1.000000 	1 1
                -10.000000 10.000000 -10.000000 1.000000 	0 1

# These next four lines describe the location of the st plane.  
# Each line describes one vertex of the quadrilateral.  The first
# four numbers of each line give the x,y,z,w coordinates of the
# corner vertex, and the last two numbers give texture coordinates,
# describing how to map u and v to the quadrilateral.
#
# The four vertices should be coplanar.

SLAB_ST		
                0.000000 -1.100000 -1.100000 1.000000 	0 0
                0.000000 -1.100000 1.100000 1.000000 	1 0
                0.000000 1.100000 1.100000 1.000000 	1 1
                0.000000 1.100000 -1.100000 1.000000 	0 1


# This line describes the dimensions of the light field array in
# u and v. (i.e. the size of the array of images.)

SAMPLES_UV	4 4

# This line describes the dimensions of the light field array in
# s and t. (i.e. the size of each image (in pixels)).

SAMPLES_ST	128 128

# This line describes the format of the images.

FORMAT		RGB

# -------------------- end of slab  --------------------


