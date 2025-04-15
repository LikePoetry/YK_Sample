#include "terrain.h"
#include <stdlib.h>
//----------------------------------------------------------------------
//Apply the erosion filter to an entire buffer
//					of height values
//----------------------------------------------------------------------
void CTERRAIN::FilterHeightField(float* fpHeightData,float fFilter)
{

}

//----------------------------------------------------------------------
// Apply the erosion filter to a single band of height values
// fpBand: the band to be filtered
// iStride: how far to advance per pass
// iCount: number of passes to make
// fFilter: the filter to apply
//----------------------------------------------------------------------
void CTERRAIN::FilterHeightBand( float* fpBand, int iStride, int iCount, float fFilter )
{

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
        //pick two points at random from the entire height map
        iRandX1 = rand() % m_iSize;
        iRandZ1 = rand() % m_iSize;

        //check to make sure that the points are not the same
		do
		{
			iRandX2= rand( )%m_iSize;
			iRandZ2= rand( )%m_iSize;
		} while ( iRandX2==iRandX1 && iRandZ2==iRandZ1 );

        //iDirX1, iDirZ1 is a vector going the same direction as the line
		iDirX1= iRandX2-iRandX1;
		iDirZ1= iRandZ2-iRandZ1;

        for( z=0; z<m_iSize; z++ )
		{
			for( x=0; x<m_iSize; x++ )
			{
				//iDirX2, iDirZ2 is a vector from iRandX1, iRandZ1 to the current point (in the loop)
				iDirX2= x-iRandX1;
				iDirZ2= z-iRandZ1;
				
				//if the result of ( iDirX2*iDirZ1 - iDirX1*iDirZ2 ) is "up" (above 0), 
				//then raise this point by iHeight
				if( ( iDirX2*iDirZ1 - iDirX1*iDirZ2 )>0 )
					fTempBuffer[( z*m_iSize )+x]+= ( float )iHeight;
			}
		}

        FilterHeightField(fTempBuffer,fFilter);
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
