#include "terrain.h"
#include <stdlib.h>
//----------------------------------------------------------------------
// Apply the erosion filter to an entire buffer
//					of height values
//----------------------------------------------------------------------
void CTERRAIN::FilterHeightField(float *fpHeightData, float fFilter)
{
    int i;
    // erode left to right
    for (i = 0; i < m_iSize; i++)
        FilterHeightBand(&fpHeightData[m_iSize * i], 1, m_iSize, fFilter);

    // erode right to left
    for (i = 0; i < m_iSize; i++)
        FilterHeightBand(&fpHeightData[m_iSize * i + m_iSize - 1], -1, m_iSize, fFilter);

    // erode top to bottom
    for (i = 0; i < m_iSize; i++)
        FilterHeightBand(&fpHeightData[i], m_iSize, m_iSize, fFilter);

    // erode from bottom to top
    for (i = 0; i < m_iSize; i++)
        FilterHeightBand(&fpHeightData[m_iSize * (m_iSize - 1) + i], -m_iSize, m_iSize, fFilter);
}

//----------------------------------------------------------------------
// Apply the erosion filter to a single band of height values
// fpBand: the band to be filtered
// iStride: how far to advance per pass
// iCount: number of passes to make
// fFilter: the filter to apply
//----------------------------------------------------------------------
void CTERRAIN::FilterHeightBand(float *fpBand, int iStride, int iCount, float fFilter)
{
    float v = fpBand[0];
    int j = iStride;
    int i;

    // go through the height band and apply the erosion filter
    for (i = 0; i < iCount - 1; i++)
    {
        fpBand[j] = fFilter * v + (1 - fFilter) * fpBand[j];

        v = fpBand[j];
        j += iStride;
    }
}

//----------------------------------------------------------------------
// 创建高度数据集
// isize 高度图的大小
// iiterations 迭代次数
// imindelta 最小高度
// imaxdelta 最大高度
// ffilter 高度过滤器
//----------------------------------------------------------------------
bool CTERRAIN::MakeTerrainFault(int iSize, int iIterations, int iMinDelta, int iMaxDelta, float fFilter)
{
    float *fTempBuffer;
    int iCurrentIteration;
    int iHeight;
    int iRandX1, iRandZ1;
    int iRandX2, iRandZ2;
    int iDirX1, iDirZ1;
    int iDirX2, iDirZ2;
    int x, z;
    int i;

    if (m_heightData.m_ucpData)
    {
        UnloadHeightMap();
    }

    m_iSize = iSize;
    // allocate the height data
    m_heightData.m_ucpData = new unsigned char[m_iSize * m_iSize];
    fTempBuffer = new float[m_iSize * m_iSize];

    if (m_heightData.m_ucpData == nullptr || fTempBuffer == nullptr)
    {
        return false;
    }

    // clear the height fTempBuffer
    for (size_t i = 0; i < m_iSize * m_iSize; i++)
    {
        fTempBuffer[i] = 0.0f;
    }

    for (iCurrentIteration = 0; iCurrentIteration < iIterations; iCurrentIteration++)
    {
        // calculate the height range (linear interpolation from iMaxDelta to
        // iMinDelta) for this fault-pass
        iHeight = iMaxDelta - ((iMaxDelta - iMinDelta) * iCurrentIteration) / iIterations;
        // pick two points at random from the entire height map
        iRandX1 = rand() % m_iSize;
        iRandZ1 = rand() % m_iSize;

        // check to make sure that the points are not the same
        do
        {
            iRandX2 = rand() % m_iSize;
            iRandZ2 = rand() % m_iSize;
        } while (iRandX2 == iRandX1 && iRandZ2 == iRandZ1);

        // iDirX1, iDirZ1 is a vector going the same direction as the line
        iDirX1 = iRandX2 - iRandX1;
        iDirZ1 = iRandZ2 - iRandZ1;

        for (z = 0; z < m_iSize; z++)
        {
            for (x = 0; x < m_iSize; x++)
            {
                // iDirX2, iDirZ2 is a vector from iRandX1, iRandZ1 to the current point (in the loop)
                iDirX2 = x - iRandX1;
                iDirZ2 = z - iRandZ1;

                // if the result of ( iDirX2*iDirZ1 - iDirX1*iDirZ2 ) is "up" (above 0),
                // then raise this point by iHeight
                if ((iDirX2 * iDirZ1 - iDirX1 * iDirZ2) > 0)
                    fTempBuffer[(z * m_iSize) + x] += (float)iHeight;
            }
        }

        FilterHeightField(fTempBuffer, fFilter);
    }

    // normalize the terrain for our purposes
    NormalizeTerrain(fTempBuffer);

    // transfer the terrain into our class's unsigned char height buffer
    for (z = 0; z < m_iSize; z++)
    {
        for (x = 0; x < m_iSize; x++)
            SetHeightAtPoint((unsigned char)fTempBuffer[(z * m_iSize) + x], x, z);
    }

    // delete temporary buffer
    if (fTempBuffer)
    {
        // delete the data
        delete[] fTempBuffer;
    }
    return true;
}

//----------------------------------------------------------------------
// Scale the terrain height values to a range of 0-255
//----------------------------------------------------------------------
void CTERRAIN::NormalizeTerrain(float *fpHeightData)
{
    float fMin, fMax;
    float fHeight;
    int i;

    fMin = fpHeightData[0];
    fMax = fpHeightData[0];

    // find the min/max values of the height fTempBuffer
    for (i = 1; i < m_iSize * m_iSize; i++)
    {
        if (fpHeightData[i] < fMin)
            fMin = fpHeightData[i];
        if (fpHeightData[i] > fMax)
            fMax = fpHeightData[i];
    }
    // find the range of the altitue
    if (fMax <= fMin)
    {
        return;
    }

    fHeight = fMax - fMin;

    // scale the values to a range of 0-255 (because I like things that way)
    for (i = 0; i < m_iSize * m_iSize; i++)
    {
        fpHeightData[i] = ((fpHeightData[i] - fMin) / fHeight) * 255.0f;
    }
}

//----------------------------------------------------------------------
// Unload the heightmap data
//----------------------------------------------------------------------
void CTERRAIN::UnloadHeightMap()
{
    if (m_heightData.m_ucpData)
    {
        delete[] m_heightData.m_ucpData;
        m_heightData.m_ucpData = nullptr;
        m_iSize = 0;
    }
}
