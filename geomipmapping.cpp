#include "geomipmapping.h"

//----------------------------------------------------------------------
// Initiate the geomapping system
// iPatchSize : the size of the patch (in vertices)
//             : a good size is usually around 17 (17*17 verts)
//----------------------------------------------------------------------
bool CGEOMIPMAPPING::Init(int iPatchSize)
{
    int x, z;
    int iLOD;
    int iDivisor;

    if (m_iSize == 0)
    {
        return false;
    }
    if (m_pPatchs)
    {
        // shutdown();
    }

    // initiate the patch information
    m_iPatchSize = iPatchSize;
    m_iNumPatchesPerSide = m_iSize / m_iPatchSize;
    m_pPatchs = new SGEOMM_PATCH[m_iNumPatchesPerSide * m_iNumPatchesPerSide];

    if (m_pPatchs == nullptr)
    {
        // allocate error;
        return false;
    }

    // figure out the maximum level of detail for a patch
    iDivisor = m_iPatchSize - 1;
    iLOD = 0;
    while (iDivisor > 2)
    {
        iDivisor = iDivisor >> 1;
        iLOD++;
    }

    // the max amout of detail
    m_iMaxLOD = iLOD;

    // initialize the patch values
    for (z = 0; z < m_iNumPatchesPerSide; z++)
    {
        for (x = 0; x < m_iNumPatchesPerSide; x++)
        {
            // initialize the patches to the lowest level of detail
            m_pPatchs[GetPatchNumber(x, z)].m_iLOD = m_iMaxLOD;
        }
    }
    return true;
}

//----------------------------------------------------------------------
// Shutdown the geomapping system
//----------------------------------------------------------------------
void CGEOMIPMAPPING::Shutdown()
{
    // delete the patch data
    if (m_pPatchs)
    {
        delete[] m_pPatchs;
        m_pPatchs = nullptr;
    }
    // reset patch values;
    m_iPatchSize = 0;
    m_iNumPatchesPerSide = 0;
}