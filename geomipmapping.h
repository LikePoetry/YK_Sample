#include "terrain.h"

struct SGEOMM_PATCH
{
    float m_fDistance;
    int m_iLOD;
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

    inline int GetPatchNumber(int PX, int PZ)
    {
        return (PZ * m_iNumPatchesPerSide + PX);
    }

    CGEOMIPMAPPING(void) {}
    ~CGEOMIPMAPPING(void) {}
};