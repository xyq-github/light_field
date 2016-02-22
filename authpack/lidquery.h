/* lidquery:
 *
 * This program reads a .lid (LIght field Description) file,
 * and returns the requested attributes.
 */


/* All of these functions return 1 on success, 0 on failure. */
int lidSLAB_COUNT   (int *result,  char *fileName);
int lidSLAB_ID      (int *result,  char *fileName, int slabno);
int lidSLAB_BASENAME(char *result, char *fileName, int slabno);
int lidSLAB_UV      (float result[4][6], char *fileName, int slabno);
int lidSLAB_UV_V    (float result[6], char *fileName, int slabno, 
		    int vertno);
int lidSLAB_UV_V_C (float *result, char *fileName, int slabno, int vertno, 
		    int component);
int lidSLAB_ST     (float result[4][6], char *fileName, int slabno);
int lidSLAB_ST_V   (float result[6], char *fileName, int slabno, 
		    int vertno);
int lidSLAB_ST_V_C (float *result, char *fileName, int slabno, int vertno, 
		    int component);
int lidSAMPLES_UV  (int result[2], char *fileName, int slabno);
int lidSAMPLES_U   (int *result, char *fileName, int slabno);
int lidSAMPLES_V   (int *result, char *fileName, int slabno);
int lidSAMPLES_ST  (int result[2], char *fileName, int slabno);
int lidSAMPLES_S   (int *result, char *fileName, int slabno);
int lidSAMPLES_T   (int *result, char *fileName, int slabno);
int lidFORMAT      (char  *result, char *fileName, int slabno);

