#pragma once

struct STRN_HEIGHT_DATA
{
    unsigned char* m_ucpData; // the height data
    int m_iSize; // the height size (must be a power of 2)
};

class CTERRAIN
{
    protected:
    STRN_HEIGHT_DATA m_heightData; // the height data

    bool MakeTerrainFault(int iSize,int iIterations,int iMinDelta,int iMaxDelta,float fFilter);
    
    void NormalizeTerrain(float* fpHeightData);
    void FilterHeightBand( float* fpBand, int iStride, int iCount, float fFilter );
    void FilterHeightField(float* fpHeightData,float fFilter);

    public:
    int m_iSize; // the size of the heightmap, must be a power of two

    void UnloadHeightMap();

    //----------------------------------------------------------------------
    // set the true height value at the given point
    //----------------------------------------------------------------------
    inline void SetHeightAtPoint(unsigned char ucHeight, int x, int z)
    {
        m_heightData.m_ucpData[(z * m_iSize) + x] = ucHeight;
    }

    CTERRAIN(){}
    ~CTERRAIN(){}
};
