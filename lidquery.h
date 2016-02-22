/* lidquery:
 *
 * This program reads a .lid (LIght field Description) file,
 * and returns the requested attributes.
 */


/* All of these functions return 1 on success, 0 on failure. */
#include <string>
using namespace std;

bool lidSLAB_COUNT   (int &result,  string fileName);
bool lidSLAB_ID      (int &result,  string fileName, int slabno);
bool lidSLAB_BASENAME(string &result, string fileName, int slabno);
bool lidSLAB_UV      (float result[4][6], string fileName, int slabno);
bool lidSLAB_UV_V    (float result[6], string fileName, int slabno,
		    int vertno);
bool lidSLAB_UV_V_C (float &result, string fileName, int slabno, int vertno,
		    int component);
bool lidSLAB_ST     (float result[4][6], string fileName, int slabno);
bool lidSLAB_ST_V   (float result[6], string fileName, int slabno,
		    int vertno);
bool lidSLAB_ST_V_C (float &result, string fileName, int slabno, int vertno,
		    int component);
bool lidSAMPLES_UV  (int result[2], string fileName, int slabno);
bool lidSAMPLES_U   (int &result, string fileName, int slabno);
bool lidSAMPLES_V   (int &result, string fileName, int slabno);
bool lidSAMPLES_ST  (int result[2], string fileName, int slabno);
bool lidSAMPLES_S   (int &result, string fileName, int slabno);
bool lidSAMPLES_T   (int &result, string fileName, int slabno);
bool lidFORMAT      (string &result, string fileName, int slabno);

