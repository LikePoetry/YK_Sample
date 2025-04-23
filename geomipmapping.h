#include "terrain.h"

struct SGEOMM_PATCH
{
    float m_fDistance;
    int m_iLOD;

    bool m_bVisible;
};

struct SGEOMM_NEIGHBOR
{
    bool m_bLeft;
    bool m_bUp;
    bool m_bRight;
    bool m_bDown;
};

class CGEOMIPMAPPING : public CTERRAIN
{
public:
    bool Init(int iPatchSize);
    void Shutdown();

    void Render();

private:
    SGEOMM_PATCH *m_pPatchs;  // the patch data
    int m_iPatchSize;         // the size of the patch (in vertices)
    int m_iNumPatchesPerSide; // the number of patches per side

    int m_iMaxLOD;

    int m_iPatchesPerFrame; // the number of rendered patches per second

    void RenderPatch(int PX, int PZ, bool bMultiTex = false, bool bDetail = false);
    void RenderFan(float cX, float cZ, float iSize, SGEOMM_NEIGHBOR neighbor, bool bMultiTex, bool bDetail);

    inline int GetPatchNumber(int PX, int PZ)
    {
        return (PZ * m_iNumPatchesPerSide + PX);
    }

    CGEOMIPMAPPING(void) {}
    ~CGEOMIPMAPPING(void) {}
};