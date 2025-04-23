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
// Render the geomapping system
//----------------------------------------------------------------------
void CGEOMIPMAPPING::Render()
{
    int x, z;

    // reset the counting variables
    m_iPatchesPerFrame = 0;

    m_iVertsPerFrame = 0;
    m_iTrisPerFrame = 0;

    // render the patches
    for (z = 0; z < m_iNumPatchesPerSide; z++)
    {
        for (x = 0; x < m_iNumPatchesPerSide; x++)
        {
            if (m_pPatchs[GetPatchNumber(x, z)].m_bVisible)
            {
                RenderPatch(x, z, true, true);
                m_iPatchesPerFrame++;
            }
        }
    }
}

//----------------------------------------------------------------------
// Render a patch of terrain
// PX : the patch x position
// PZ : the patch z position
// bMultiTex : use multitexturing or not
// bDetail : render with a detail map or not
//----------------------------------------------------------------------
void CGEOMIPMAPPING::RenderPatch(int PX, int PZ, bool bMultiTex, bool bDetail)
{
    SGEOMM_NEIGHBOR patchNeighbor;
    SGEOMM_NEIGHBOR fanNeighbor;
    float fSize;
    float fHalfSize;
    float x, z;

    int iPatch = GetPatchNumber(PX, PZ);
    int iDivisor;
    int iLOD;

    // find out information about the patch to the current patch's left, if the patch is of a
    // greater detail or there is no patch to the left, we can render the mid-left vertex
    if (m_pPatchs[GetPatchNumber(PX - 1, PZ)].m_iLOD <= m_pPatchs[iPatch].m_iLOD || PX == 0)
    {
        patchNeighbor.m_bLeft = true;
    }
    else
    {
        patchNeighbor.m_bLeft = false;
    }

    if (m_pPatchs[GetPatchNumber(PX, PZ + 1)].m_iLOD <= m_pPatchs[iPatch].m_iLOD || PZ == m_iNumPatchesPerSide)
    {
        patchNeighbor.m_bUp = true;
    }
    else
    {
        patchNeighbor.m_bUp = false;
    }

    if (m_pPatchs[GetPatchNumber(PX + 1, PZ)].m_iLOD <= m_pPatchs[iPatch].m_iLOD || PX == m_iNumPatchesPerSide)
    {
        patchNeighbor.m_bRight = true;
    }
    else
    {
        patchNeighbor.m_bRight = false;
    }

    if (m_pPatchs[GetPatchNumber(PX, PZ - 1)].m_iLOD <= m_pPatchs[iPatch].m_iLOD || PZ == 0)
    {
        patchNeighbor.m_bDown = true;
    }
    else
    {
        patchNeighbor.m_bDown = false;
    }

    // we need to determine the distance between each triangle-fan that
    // we will be rendering
    iLOD = m_pPatchs[GetPatchNumber(PX, PZ)].m_iLOD + 1;
    fSize = (float)m_iPatchSize;
    iDivisor = m_iPatchSize - 1;

    // find out how many fan divisions we are going to have
    while (--iLOD > -1)
        iDivisor = iDivisor >> 1;

    // the size between the center of each triangle fan
    fSize /= iDivisor;

    // half the size between the center of each triangle fan (this will be
    // the size between each vertex)
    fHalfSize = fSize / 2.0f;

    for (z = fHalfSize; ((int)(z + fHalfSize)) < m_iPatchSize + 1; z += fSize)
    {
        for (x = fHalfSize; ((int)(x + fHalfSize)) < m_iPatchSize + 1; x += fSize)
        {
            // if the fan is in the left row, we may need to adjust it's rendering to prevent cracks
            if (x == fHalfSize)
            {
                fanNeighbor.m_bLeft = patchNeighbor.m_bLeft; 
            }else
            {
                fanNeighbor.m_bLeft = true;
            }

            // if the fan is in the bottom row, we may need to adjust it's rendering to prevent cracks
            if (z == fHalfSize)
            {
                fanNeighbor.m_bDown = patchNeighbor.m_bDown;
            }else
            {
                fanNeighbor.m_bDown = true;
            }

            // if the fan is in the right row, we may need to adjust it's rendering to prevent cracks
            if(x>=(m_iPatchSize-fHalfSize))
            {
                fanNeighbor.m_bRight = patchNeighbor.m_bRight;
            }else   
            {
                fanNeighbor.m_bRight = true;
            }

            // if the fan is in the top row, we may need to adjust it's rendering to prevent cracks
            if(z>=(m_iPatchSize-fHalfSize))
            {
                fanNeighbor.m_bUp = patchNeighbor.m_bUp;
            }else
            {
                fanNeighbor.m_bUp = true;
            }

            // render the triangle fan
            RenderFan((PX*m_iPatchSize) + x, (PZ*m_iPatchSize) + z, fSize,fanNeighbor, bMultiTex, bDetail);

        }
    }
}

//----------------------------------------------------------------------
// Render a triangle fan
//----------------------------------------------------------------------
void CGEOMIPMAPPING::RenderFan(float cX,float cZ,float fSize,SGEOMM_NEIGHBOR neighbor,bool bMultiTex,bool bDetail)
{
    float fTexLeft,fTexBottom,fMidX,fMidZ,fTexRight,fTexTop;
    float fHalfSize = fSize / 2.0f;

    // find out the texture coordinates for the triangle fan
    //  -------------------------
    //  |          |            |
    //  |          |            |
    //  |          |            |
    //  |          |  cx cz     |
    //  -------------------------  fHalfsize
    //  |          |            |
    //  |          |            |
    //  |          |            |
    //  |          |            |
    //  -------------------------

    //(cx,cz),(cx-fHalfSize,cz-fHalfsize),(cx-fHalfSize,cz),(cx-fHalfSize,cz+fHalfSize).....
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