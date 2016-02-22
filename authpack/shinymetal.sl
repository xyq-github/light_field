/* @(#)shinymetal.sl	1.2	(Pixar - RenderMan Division)	9/28/89 */

/*---------------------------------------------------------------------
 *
 * Copyright (c) 1988 Pixar.   All rights reserved.   This data file and
 * the model it represents contains  proprietary confidential information
 * and trade secrets of Pixar.  Reverse engineering of binary code
 * is prohibited.   Use of copyright  notice is precautionary  and
 * does not imply publication.
 *
 *                    RESTRICTED RIGHTS NOTICE
 *
 * Use,  duplication,  or disclosure by the Government is subject to
 * restrictions as set forth in subdivision (b)(3)(ii) of the Rights
 * in Technical Data and Computer Software clause at 252.227-7013.
 *
 * Pixar
 * 3240 Kerner Blvd.
 * San Rafael, CA  94901
 *
 *--------------------------------------------------------------------*/

surface
shinymetal (float Ka=0, Ks=1, Kr = 1, Kd=1,roughness=.1; color diffusecolor=1;
	    string texturename = "";)
{
    point Nf, D, V;
    color Cr;

    Nf = faceforward(normalize(N), I) ;
    V = normalize(-I);
    D = reflect(I, Nf) ;
    D = transform( "world" , point "world" (0,0,0) + D);

    if (texturename != "") {
	Cr = Kr * color environment(texturename, D);
    } else {
	Cr = 0.;
    } 
    
    Oi = Os;
    Ci = Os * Ks * ( (.95-Cs)*(Ka*ambient() + Kd*diffusecolor*diffuse(Nf)) +
	Cs*specular(Nf,V,roughness) + Cs*Cr);
}
