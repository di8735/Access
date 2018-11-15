#include "CModel.h"

CModel::CModel()
{
	D3DXMatrixIdentity(&worldMatrix);
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixIdentity(&scaleMatrix);
	mIsEffect = false;
	mCurrAnimNum = 0;
	mAnimTime = 0.0f;
}

CModel::~CModel()
{
}

IDirect3DVertexShader9*     g_pIndexedVertexShader[4];
ID3DXEffect*                g_pEffect = NULL;       // D3DX effect interface
D3DXMATRIXA16               g_matView;              // View matrix
DWORD                       g_dwBehaviorFlags;      // Behavior flags of the 3D device
METHOD                      g_SkinningMethod = D3DNONINDEXED; // Current skinning method
D3DXMATRIXA16*              g_pBoneMatrices = NULL;
UINT                        g_NumBoneMatricesMax = 0;
bool                        g_bUseSoftwareVP;       // Flag to indicate whether software vp is

void CModel::DrawMeshContainer(IDirect3DDevice9* pd3dDevice, LPD3DXMESHCONTAINER pMeshContainerBase, LPD3DXFRAME pFrameBase)
{
	//HRESULT hr;
	D3DXMESHCONTAINER_DERIVED* pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainerBase;
	D3DXFRAME_DERIVED* pFrame = (D3DXFRAME_DERIVED*)pFrameBase;
	UINT iMaterial;
	UINT NumBlend;
	UINT iAttrib;
	DWORD AttribIdPrev;
	LPD3DXBONECOMBINATION pBoneComb;

	UINT iMatrixIndex;
	UINT iPaletteEntry;
	D3DXMATRIXA16 matTemp;
	D3DCAPS9 d3dCaps;
	pd3dDevice->GetDeviceCaps(&d3dCaps);

	// first check for skinning
	if (pMeshContainer->pSkinInfo != NULL)
	{
		if (g_SkinningMethod == D3DNONINDEXED)
		{
			AttribIdPrev = UNUSED32;
			pBoneComb = reinterpret_cast<LPD3DXBONECOMBINATION>(pMeshContainer->pBoneCombinationBuf->GetBufferPointer
				());

			// Draw using default vtx processing of the device (typically HW)
			for (iAttrib = 0; iAttrib < pMeshContainer->NumAttributeGroups; iAttrib++)
			{
				NumBlend = 0;
				for (DWORD i = 0; i < pMeshContainer->NumInfl; ++i)
				{
					if (pBoneComb[iAttrib].BoneId[i] != UINT_MAX)
					{
						NumBlend = i;
					}
				}
				////////////////
				if (d3dCaps.MaxVertexBlendMatrices >= NumBlend + 1)
				{
					// first calculate the world matrices for the current set of blend weights and get the accurate count of the number of blends
					for (DWORD i = 0; i < pMeshContainer->NumInfl; ++i)
					{
						iMatrixIndex = pBoneComb[iAttrib].BoneId[i];
						if (iMatrixIndex != UINT_MAX)
						{
							D3DXMatrixMultiply(&matTemp, &pMeshContainer->pBoneOffsetMatrices[iMatrixIndex],
								pMeshContainer->ppBoneMatrixPtrs[iMatrixIndex]);
							pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(i), &matTemp);
						}
					}

					pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, NumBlend);

					// lookup the material used for this subset of faces
					if ((AttribIdPrev != pBoneComb[iAttrib].AttribId) || (AttribIdPrev == UNUSED32))
					{
						pd3dDevice->SetMaterial(&pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D);
						pd3dDevice->SetTexture(0, pMeshContainer->ppTextures[pBoneComb[iAttrib].AttribId]);
						AttribIdPrev = pBoneComb[iAttrib].AttribId;
					}

					// draw the subset now that the correct material and matrices are loaded
					pMeshContainer->MeshData.pMesh->DrawSubset(iAttrib);
				}
				///////////////////////
			}

			// If necessary, draw parts that HW could not handle using SW
			if (pMeshContainer->iAttributeSW < pMeshContainer->NumAttributeGroups)
			{
				AttribIdPrev = UNUSED32;
				pd3dDevice->SetSoftwareVertexProcessing(TRUE);
				for (iAttrib = pMeshContainer->iAttributeSW; iAttrib < pMeshContainer->NumAttributeGroups; iAttrib++)
				{
					NumBlend = 0;
					for (DWORD i = 0; i < pMeshContainer->NumInfl; ++i)
					{
						if (pBoneComb[iAttrib].BoneId[i] != UINT_MAX)
						{
							NumBlend = i;
						}
					}

					if (d3dCaps.MaxVertexBlendMatrices < NumBlend + 1)
					{
						// first calculate the world matrices for the current set of blend weights and get the accurate count of the number of blends
						for (DWORD i = 0; i < pMeshContainer->NumInfl; ++i)
						{
							iMatrixIndex = pBoneComb[iAttrib].BoneId[i];
							if (iMatrixIndex != UINT_MAX)
							{
								D3DXMatrixMultiply(&matTemp, &pMeshContainer->pBoneOffsetMatrices[iMatrixIndex],
									pMeshContainer->ppBoneMatrixPtrs[iMatrixIndex]);
								pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(i), &matTemp);
							}
						}

						pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, NumBlend);

						// lookup the material used for this subset of faces
						if ((AttribIdPrev != pBoneComb[iAttrib].AttribId) || (AttribIdPrev == UNUSED32))
						{
							pd3dDevice->SetMaterial(&pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D);
							pd3dDevice->SetTexture(0, pMeshContainer->ppTextures[pBoneComb[iAttrib].AttribId]);
							AttribIdPrev = pBoneComb[iAttrib].AttribId;
						}

						// draw the subset now that the correct material and matrices are loaded
						pMeshContainer->MeshData.pMesh->DrawSubset(iAttrib);
					}
				}
				pd3dDevice->SetSoftwareVertexProcessing(FALSE);
			}

			pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, 0);
		}
		else if (g_SkinningMethod == D3DINDEXED)
		{
			// if hw doesn't support indexed vertex processing, switch to software vertex processing
			if (pMeshContainer->UseSoftwareVP)
			{
				// If hw or pure hw vertex processing is forced, we can't render the
				// mesh, so just exit out.  Typical applications should create
				// a device with appropriate vertex processing capability for this
				// skinning method.
				if (g_dwBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)
					return;

				pd3dDevice->SetSoftwareVertexProcessing(TRUE);
			}

			// set the number of vertex blend indices to be blended
			if (pMeshContainer->NumInfl == 1)
			{
				pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_0WEIGHTS);
			}
			else
			{
				pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, pMeshContainer->NumInfl - 1);
			}

			if (pMeshContainer->NumInfl)
				pd3dDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, TRUE);

			// for each attribute group in the mesh, calculate the set of matrices in the palette and then draw the mesh subset
			pBoneComb = reinterpret_cast<LPD3DXBONECOMBINATION>(pMeshContainer->pBoneCombinationBuf->GetBufferPointer
				());
			for (iAttrib = 0; iAttrib < pMeshContainer->NumAttributeGroups; iAttrib++)
			{
				// first calculate all the world matrices
				for (iPaletteEntry = 0; iPaletteEntry < pMeshContainer->NumPaletteEntries; ++iPaletteEntry)
				{
					iMatrixIndex = pBoneComb[iAttrib].BoneId[iPaletteEntry];
					if (iMatrixIndex != UINT_MAX)
					{
						D3DXMatrixMultiply(&matTemp, &pMeshContainer->pBoneOffsetMatrices[iMatrixIndex],
							pMeshContainer->ppBoneMatrixPtrs[iMatrixIndex]);
						pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(iPaletteEntry), &matTemp);
					}
				}

				// setup the material of the mesh subset - REMEMBER to use the original pre-skinning attribute id to get the correct material id
				pd3dDevice->SetMaterial(&pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D);
				pd3dDevice->SetTexture(0, pMeshContainer->ppTextures[pBoneComb[iAttrib].AttribId]);

				// finally draw the subset with the current world matrix palette and material state
				pMeshContainer->MeshData.pMesh->DrawSubset(iAttrib);
			}

			// reset blending state
			pd3dDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
			pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, 0);

			// remember to reset back to hw vertex processing if software was required
			if (pMeshContainer->UseSoftwareVP)
			{
				pd3dDevice->SetSoftwareVertexProcessing(FALSE);
			}
		}
		else if (g_SkinningMethod == D3DINDEXEDVS)
		{
			// Use COLOR instead of UBYTE4 since Geforce3 does not support it
			// vConst.w should be 3, but due to COLOR/UBYTE4 issue, mul by 255 and add epsilon
			D3DXVECTOR4 vConst(1.0f, 0.0f, 0.0f, 765.01f);

			if (pMeshContainer->UseSoftwareVP)
			{
				// If hw or pure hw vertex processing is forced, we can't render the
				// mesh, so just exit out.  Typical applications should create
				// a device with appropriate vertex processing capability for this
				// skinning method.
				if (g_dwBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)
					return;

				pd3dDevice->SetSoftwareVertexProcessing(TRUE);
			}

			pd3dDevice->SetVertexShader(g_pIndexedVertexShader[pMeshContainer->NumInfl - 1]);

			pBoneComb = reinterpret_cast<LPD3DXBONECOMBINATION>(pMeshContainer->pBoneCombinationBuf->GetBufferPointer
				());
			for (iAttrib = 0; iAttrib < pMeshContainer->NumAttributeGroups; iAttrib++)
			{
				// first calculate all the world matrices
				for (iPaletteEntry = 0; iPaletteEntry < pMeshContainer->NumPaletteEntries; ++iPaletteEntry)
				{
					iMatrixIndex = pBoneComb[iAttrib].BoneId[iPaletteEntry];
					if (iMatrixIndex != UINT_MAX)
					{
						D3DXMatrixMultiply(&matTemp, &pMeshContainer->pBoneOffsetMatrices[iMatrixIndex],
							pMeshContainer->ppBoneMatrixPtrs[iMatrixIndex]);
						D3DXMatrixMultiplyTranspose(&matTemp, &matTemp, &g_matView);
						pd3dDevice->SetVertexShaderConstantF(iPaletteEntry * 3 + 9, (float*)&matTemp, 3);
					}
				}

				// Sum of all ambient and emissive contribution
				D3DXCOLOR color1(pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D.Ambient);
				D3DXCOLOR color2(.25, .25, .25, 1.0);
				D3DXCOLOR ambEmm;
				D3DXColorModulate(&ambEmm, &color1, &color2);
				ambEmm += D3DXCOLOR(pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D.Emissive);

				// set material color properties 
				pd3dDevice->SetVertexShaderConstantF(8,
					(float*)&(
					pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D.Diffuse), 1);
				pd3dDevice->SetVertexShaderConstantF(7, (float*)&ambEmm, 1);
				vConst.y = pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D.Power;
				pd3dDevice->SetVertexShaderConstantF(0, (float*)&vConst, 1);

				pd3dDevice->SetTexture(0, pMeshContainer->ppTextures[pBoneComb[iAttrib].AttribId]);

				// finally draw the subset with the current world matrix palette and material state
				pMeshContainer->MeshData.pMesh->DrawSubset(iAttrib);
			}

			// remember to reset back to hw vertex processing if software was required
			if (pMeshContainer->UseSoftwareVP)
			{
				pd3dDevice->SetSoftwareVertexProcessing(FALSE);
			}
			pd3dDevice->SetVertexShader(NULL);
		}
		else if (g_SkinningMethod == D3DINDEXEDHLSLVS)
		{
			if (pMeshContainer->UseSoftwareVP)
			{
				// If hw or pure hw vertex processing is forced, we can't render the
				// mesh, so just exit out.  Typical applications should create
				// a device with appropriate vertex processing capability for this
				// skinning method.
				if (g_dwBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)
					return;

				pd3dDevice->SetSoftwareVertexProcessing(TRUE);
			}

			pBoneComb = reinterpret_cast<LPD3DXBONECOMBINATION>(pMeshContainer->pBoneCombinationBuf->GetBufferPointer
				());
			for (iAttrib = 0; iAttrib < pMeshContainer->NumAttributeGroups; iAttrib++)
			{
				// first calculate all the world matrices
				for (iPaletteEntry = 0; iPaletteEntry < pMeshContainer->NumPaletteEntries; ++iPaletteEntry)
				{
					iMatrixIndex = pBoneComb[iAttrib].BoneId[iPaletteEntry];
					if (iMatrixIndex != UINT_MAX)
					{
						D3DXMatrixMultiply(&matTemp, &pMeshContainer->pBoneOffsetMatrices[iMatrixIndex],
							pMeshContainer->ppBoneMatrixPtrs[iMatrixIndex]);
						D3DXMatrixMultiply(&g_pBoneMatrices[iPaletteEntry], &matTemp, &g_matView);
					}
				}
				g_pEffect->SetMatrixArray("mWorldMatrixArray", g_pBoneMatrices,
					pMeshContainer->NumPaletteEntries);

				// Sum of all ambient and emissive contribution
				D3DXCOLOR color1(pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D.Ambient);
				D3DXCOLOR color2(.25, .25, .25, 1.0);
				D3DXCOLOR ambEmm;
				D3DXColorModulate(&ambEmm, &color1, &color2);
				ambEmm += D3DXCOLOR(pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D.Emissive);

				// set material color properties 
				g_pEffect->SetVector("MaterialDiffuse",
					(D3DXVECTOR4*)&(
					pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D.Diffuse));
				g_pEffect->SetVector("MaterialAmbient", (D3DXVECTOR4*)&ambEmm);

				// setup the material of the mesh subset - REMEMBER to use the original pre-skinning attribute id to get the correct material id
				pd3dDevice->SetTexture(0, pMeshContainer->ppTextures[pBoneComb[iAttrib].AttribId]);

				// Set CurNumBones to select the correct vertex shader for the number of bones
				g_pEffect->SetInt("CurNumBones", pMeshContainer->NumInfl - 1);

				// Start the effect now all parameters have been updated
				UINT numPasses;
				g_pEffect->Begin(&numPasses, D3DXFX_DONOTSAVESTATE);
				for (UINT iPass = 0; iPass < numPasses; iPass++)
				{
					g_pEffect->BeginPass(iPass);

					// draw the subset with the current world matrix palette and material state
					pMeshContainer->MeshData.pMesh->DrawSubset(iAttrib);

					g_pEffect->EndPass();
				}

				g_pEffect->End();

				pd3dDevice->SetVertexShader(NULL);
			}

			// remember to reset back to hw vertex processing if software was required
			if (pMeshContainer->UseSoftwareVP)
			{
				pd3dDevice->SetSoftwareVertexProcessing(FALSE);
			}
		}
		else if (g_SkinningMethod == SOFTWARE)
		{
			D3DXMATRIX Identity;
			DWORD cBones = pMeshContainer->pSkinInfo->GetNumBones();
			DWORD iBone;
			PBYTE pbVerticesSrc;
			PBYTE pbVerticesDest;

			// set up bone transforms
			for (iBone = 0; iBone < cBones; ++iBone)
			{
				D3DXMatrixMultiply
					(
					&g_pBoneMatrices[iBone],                 // output
					&pMeshContainer->pBoneOffsetMatrices[iBone],
					pMeshContainer->ppBoneMatrixPtrs[iBone]
					);
			}

			// set world transform
			D3DXMatrixIdentity(&Identity);
			pd3dDevice->SetTransform(D3DTS_WORLD, &Identity);

			pMeshContainer->pOrigMesh->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&pbVerticesSrc);
			pMeshContainer->MeshData.pMesh->LockVertexBuffer(0, (LPVOID*)&pbVerticesDest);

			// generate skinned mesh
			pMeshContainer->pSkinInfo->UpdateSkinnedMesh(g_pBoneMatrices, NULL, pbVerticesSrc, pbVerticesDest);

			pMeshContainer->pOrigMesh->UnlockVertexBuffer();
			pMeshContainer->MeshData.pMesh->UnlockVertexBuffer();

			for (iAttrib = 0; iAttrib < pMeshContainer->NumAttributeGroups; iAttrib++)
			{
				pd3dDevice->SetMaterial(&(
					pMeshContainer->pMaterials[pMeshContainer->pAttributeTable[iAttrib].AttribId].MatD3D));
				pd3dDevice->SetTexture(0,
					pMeshContainer->ppTextures[pMeshContainer->pAttributeTable[iAttrib].AttribId]);
				pMeshContainer->MeshData.pMesh->DrawSubset(pMeshContainer->pAttributeTable[iAttrib].AttribId);
			}
		}
		else // bug out as unsupported mode
		{
			return;
		}
	}
	else  // standard mesh, just draw it after setting material properties
	{
		pd3dDevice->SetTransform(D3DTS_WORLD, &pFrame->CombinedTransformationMatrix);

		for (iMaterial = 0; iMaterial < pMeshContainer->NumMaterials; iMaterial++)
		{
			if (mIsEffect)
				pd3dDevice->SetMaterial(&d3d::WHITE_MTRL);
			else
				pd3dDevice->SetMaterial(&pMeshContainer->pMaterials[iMaterial].MatD3D);
			pd3dDevice->SetTexture(0, pMeshContainer->ppTextures[iMaterial]);
			pMeshContainer->MeshData.pMesh->DrawSubset(iMaterial);
		}
	}
}

void CModel::DrawFrame(IDirect3DDevice9* pd3dDevice, LPD3DXFRAME pFrame)
{
	LPD3DXMESHCONTAINER pMeshContainer;

	pMeshContainer = pFrame->pMeshContainer;
	while (pMeshContainer != NULL)
	{
		DrawMeshContainer(pd3dDevice, pMeshContainer, pFrame);

		pMeshContainer = pMeshContainer->pNextMeshContainer;
	}

	if (pFrame->pFrameSibling != NULL)
	{
		DrawFrame(pd3dDevice, pFrame->pFrameSibling);
	}

	if (pFrame->pFrameFirstChild != NULL)
	{
		DrawFrame(pd3dDevice, pFrame->pFrameFirstChild);
	}
}

HRESULT CModel::SetupBoneMatrixPointers(LPD3DXFRAME pFrame)
{
	HRESULT hr;
	if (pFrame->pMeshContainer != NULL)
	{
		hr = SetupBoneMatrixPointersOnMesh(pFrame->pMeshContainer);
		if (FAILED(hr))
			return hr;
	}

	if (pFrame->pFrameSibling != NULL)
	{
		hr = SetupBoneMatrixPointers(pFrame->pFrameSibling);
		if (FAILED(hr))
			return hr;
	}

	if (pFrame->pFrameFirstChild != NULL)
	{
		hr = SetupBoneMatrixPointers(pFrame->pFrameFirstChild);
		if (FAILED(hr))
			return hr;
	}

	return S_OK;
}

HRESULT CModel::SetupBoneMatrixPointersOnMesh(LPD3DXMESHCONTAINER pMeshContainerBase)
{
	UINT iBone, cBones;
	D3DXFRAME_DERIVED* pFrame;

	D3DXMESHCONTAINER_DERIVED* pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainerBase;

	// if there is a skinmesh, then setup the bone matrices
	
	if (pMeshContainer->pSkinInfo != NULL)
	{
		cBones = pMeshContainer->pSkinInfo->GetNumBones();

		pMeshContainer->ppBoneMatrixPtrs = new D3DXMATRIX*[cBones];
		if (pMeshContainer->ppBoneMatrixPtrs == NULL)
			return E_OUTOFMEMORY;

		for (iBone = 0; iBone < cBones; iBone++)
		{
			pFrame = (D3DXFRAME_DERIVED*)D3DXFrameFind(m_pFrameRoot,
				pMeshContainer->pSkinInfo->GetBoneName(iBone));

			if (pFrame == NULL)
				return E_FAIL;

			pMeshContainer->ppBoneMatrixPtrs[iBone] = &pFrame->CombinedTransformationMatrix;
		}
	}
	return S_OK;
}

void CModel::LoadXFile(wchar_t* strFileName, LPDIRECT3DDEVICE9 pd3dDevice)
{
	HRESULT hr;
	hr = D3DXLoadMeshHierarchyFromX(strFileName, D3DXMESH_MANAGED, pd3dDevice, &Alloc, NULL, &m_pFrameRoot, &m_pAnimController);
	if (hr == D3DERR_INVALIDCALL)
	{
		::MessageBox(0, L"D3DXLoadMeshHierarchyFromX() - FAILED", 0, 0);
	}
	SetupBoneMatrixPointers(m_pFrameRoot);
}

void CModel::SelectAnimation(int n)
{
	LPD3DXANIMATIONSET pAnimSet;
	m_pAnimController->GetAnimationSet(n, &pAnimSet);
	m_pAnimController->SetTrackAnimationSet(0, pAnimSet);
	m_pAnimController->ResetTime();
	mCurrAnimNum = n;
}

void CModel::selectAnimationPlayOnce(int n, float timeDelta)
{
	LPD3DXANIMATIONSET pAnimSet;
	m_pAnimController->GetAnimationSet(n, &pAnimSet);
	m_pAnimController->SetTrackAnimationSet(0, pAnimSet);
	m_pAnimController->ResetTime();
	mCurrAnimNum = n;
	double d = pAnimSet->GetPeriod();
	if (mAnimTime > pAnimSet->GetPeriod() - 0.05f)
	{
		m_pAnimController->SetTrackPosition(0, pAnimSet->GetPeriod() - 0.05f);
	}
	mAnimTime += timeDelta;
}

HRESULT GenerateSkinnedMesh(IDirect3DDevice9* pd3dDevice, D3DXMESHCONTAINER_DERIVED* pMeshContainer)
{
	HRESULT hr = S_OK;
	D3DCAPS9 d3dCaps;
	pd3dDevice->GetDeviceCaps(&d3dCaps);

	if (pMeshContainer->pSkinInfo == NULL)
		return hr;

	g_bUseSoftwareVP = false;

	SAFE_RELEASE(pMeshContainer->MeshData.pMesh);
	SAFE_RELEASE(pMeshContainer->pBoneCombinationBuf);

	// if non-indexed skinning mode selected, use ConvertToBlendedMesh to generate drawable mesh
	if (g_SkinningMethod == D3DNONINDEXED)
	{

		hr = pMeshContainer->pSkinInfo->ConvertToBlendedMesh
			(
			pMeshContainer->pOrigMesh,
			D3DXMESH_MANAGED | D3DXMESHOPT_VERTEXCACHE,
			pMeshContainer->pAdjacency,
			NULL, NULL, NULL,
			&pMeshContainer->NumInfl,
			&pMeshContainer->NumAttributeGroups,
			&pMeshContainer->pBoneCombinationBuf,
			&pMeshContainer->MeshData.pMesh
			);
		if (FAILED(hr))
			goto e_Exit;


		// If the device can only do 2 matrix blends, ConvertToBlendedMesh cannot approximate all meshes to it
		// Thus we split the mesh in two parts: The part that uses at most 2 matrices and the rest. The first is
		// drawn using the device's HW vertex processing and the rest is drawn using SW vertex processing.
		LPD3DXBONECOMBINATION rgBoneCombinations = reinterpret_cast<LPD3DXBONECOMBINATION>(
			pMeshContainer->pBoneCombinationBuf->GetBufferPointer());

		// look for any set of bone combinations that do not fit the caps
		for (pMeshContainer->iAttributeSW = 0; pMeshContainer->iAttributeSW < pMeshContainer->NumAttributeGroups;
			pMeshContainer->iAttributeSW++)
		{
			DWORD cInfl = 0;

			for (DWORD iInfl = 0; iInfl < pMeshContainer->NumInfl; iInfl++)
			{
				if (rgBoneCombinations[pMeshContainer->iAttributeSW].BoneId[iInfl] != UINT_MAX)
				{
					++cInfl;
				}
			}

			if (cInfl > d3dCaps.MaxVertexBlendMatrices)
			{
				break;
			}
		}

		// if there is both HW and SW, add the Software Processing flag
		if (pMeshContainer->iAttributeSW < pMeshContainer->NumAttributeGroups)
		{
			LPD3DXMESH pMeshTmp;

			hr = pMeshContainer->MeshData.pMesh->CloneMeshFVF(D3DXMESH_SOFTWAREPROCESSING |
				pMeshContainer->MeshData.pMesh->GetOptions(),
				pMeshContainer->MeshData.pMesh->GetFVF(),
				pd3dDevice, &pMeshTmp);
			if (FAILED(hr))
			{
				goto e_Exit;
			}

			pMeshContainer->MeshData.pMesh->Release();
			pMeshContainer->MeshData.pMesh = pMeshTmp;
			pMeshTmp = NULL;
		}
	}
	// if indexed skinning mode selected, use ConvertToIndexedsBlendedMesh to generate drawable mesh
	else if (g_SkinningMethod == D3DINDEXED)
	{
		DWORD NumMaxFaceInfl;
		DWORD Flags = D3DXMESHOPT_VERTEXCACHE;

		LPDIRECT3DINDEXBUFFER9 pIB;
		hr = pMeshContainer->pOrigMesh->GetIndexBuffer(&pIB);
		if (FAILED(hr))
			goto e_Exit;

		hr = pMeshContainer->pSkinInfo->GetMaxFaceInfluences(pIB,
			pMeshContainer->pOrigMesh->GetNumFaces(),
			&NumMaxFaceInfl);
		pIB->Release();
		if (FAILED(hr))
			goto e_Exit;

		// 12 entry palette guarantees that any triangle (4 independent influences per vertex of a tri)
		// can be handled
		NumMaxFaceInfl = min(NumMaxFaceInfl, 12);

		if (d3dCaps.MaxVertexBlendMatrixIndex + 1 < NumMaxFaceInfl)
		{
			// HW does not support indexed vertex blending. Use SW instead
			pMeshContainer->NumPaletteEntries = min(256, pMeshContainer->pSkinInfo->GetNumBones());
			pMeshContainer->UseSoftwareVP = true;
			g_bUseSoftwareVP = true;
			Flags |= D3DXMESH_SYSTEMMEM;
		}
		else
		{
			// using hardware - determine palette size from caps and number of bones
			// If normals are present in the vertex data that needs to be blended for lighting, then 
			// the number of matrices is half the number specified by MaxVertexBlendMatrixIndex.
			pMeshContainer->NumPaletteEntries = min((d3dCaps.MaxVertexBlendMatrixIndex + 1) / 2,
				pMeshContainer->pSkinInfo->GetNumBones());
			pMeshContainer->UseSoftwareVP = false;
			Flags |= D3DXMESH_MANAGED;
		}

		hr = pMeshContainer->pSkinInfo->ConvertToIndexedBlendedMesh
			(
			pMeshContainer->pOrigMesh,
			Flags,
			pMeshContainer->NumPaletteEntries,
			pMeshContainer->pAdjacency,
			NULL, NULL, NULL,
			&pMeshContainer->NumInfl,
			&pMeshContainer->NumAttributeGroups,
			&pMeshContainer->pBoneCombinationBuf,
			&pMeshContainer->MeshData.pMesh);
		if (FAILED(hr))
			goto e_Exit;
	}
	// if vertex shader indexed skinning mode selected, use ConvertToIndexedsBlendedMesh to generate drawable mesh
	else if ((g_SkinningMethod == D3DINDEXEDVS) || (g_SkinningMethod == D3DINDEXEDHLSLVS))
	{
		// Get palette size
		// First 9 constants are used for other data.  Each 4x3 matrix takes up 3 constants.
		// (96 - 9) /3 i.e. Maximum constant count - used constants 
		UINT MaxMatrices = 26;
		pMeshContainer->NumPaletteEntries = min(MaxMatrices, pMeshContainer->pSkinInfo->GetNumBones());

		DWORD Flags = D3DXMESHOPT_VERTEXCACHE;
		if (d3dCaps.VertexShaderVersion >= D3DVS_VERSION(1, 1))
		{
			pMeshContainer->UseSoftwareVP = false;
			Flags |= D3DXMESH_MANAGED;
		}
		else
		{
			pMeshContainer->UseSoftwareVP = true;
			g_bUseSoftwareVP = true;
			Flags |= D3DXMESH_SYSTEMMEM;
		}

		SAFE_RELEASE(pMeshContainer->MeshData.pMesh);

		hr = pMeshContainer->pSkinInfo->ConvertToIndexedBlendedMesh
			(
			pMeshContainer->pOrigMesh,
			Flags,
			pMeshContainer->NumPaletteEntries,
			pMeshContainer->pAdjacency,
			NULL, NULL, NULL,
			&pMeshContainer->NumInfl,
			&pMeshContainer->NumAttributeGroups,
			&pMeshContainer->pBoneCombinationBuf,
			&pMeshContainer->MeshData.pMesh);
		if (FAILED(hr))
			goto e_Exit;


		// FVF has to match our declarator. Vertex shaders are not as forgiving as FF pipeline
		DWORD NewFVF = (pMeshContainer->MeshData.pMesh->GetFVF() & D3DFVF_POSITION_MASK) | D3DFVF_NORMAL |
			D3DFVF_TEX1 | D3DFVF_LASTBETA_UBYTE4;
		if (NewFVF != pMeshContainer->MeshData.pMesh->GetFVF())
		{
			LPD3DXMESH pMesh;
			hr = pMeshContainer->MeshData.pMesh->CloneMeshFVF(pMeshContainer->MeshData.pMesh->GetOptions(), NewFVF,
				pd3dDevice, &pMesh);
			if (!FAILED(hr))
			{
				pMeshContainer->MeshData.pMesh->Release();
				pMeshContainer->MeshData.pMesh = pMesh;
				pMesh = NULL;
			}
		}

		D3DVERTEXELEMENT9 pDecl[MAX_FVF_DECL_SIZE];
		LPD3DVERTEXELEMENT9 pDeclCur;
		hr = pMeshContainer->MeshData.pMesh->GetDeclaration(pDecl);
		if (FAILED(hr))
			goto e_Exit;

		// the vertex shader is expecting to interpret the UBYTE4 as a D3DCOLOR, so update the type 
		//   NOTE: this cannot be done with CloneMesh, that would convert the UBYTE4 data to float and then to D3DCOLOR
		//          this is more of a "cast" operation
		pDeclCur = pDecl;
		while (pDeclCur->Stream != 0xff)
		{
			if ((pDeclCur->Usage == D3DDECLUSAGE_BLENDINDICES) && (pDeclCur->UsageIndex == 0))
				pDeclCur->Type = D3DDECLTYPE_D3DCOLOR;
			pDeclCur++;
		}

		hr = pMeshContainer->MeshData.pMesh->UpdateSemantics(pDecl);
		if (FAILED(hr))
			goto e_Exit;

		// allocate a buffer for bone matrices, but only if another mesh has not allocated one of the same size or larger
		if (g_NumBoneMatricesMax < pMeshContainer->pSkinInfo->GetNumBones())
		{
			g_NumBoneMatricesMax = pMeshContainer->pSkinInfo->GetNumBones();

			// Allocate space for blend matrices
			delete[] g_pBoneMatrices;
			g_pBoneMatrices = new D3DXMATRIXA16[g_NumBoneMatricesMax];
			if (g_pBoneMatrices == NULL)
			{
				hr = E_OUTOFMEMORY;
				goto e_Exit;
			}
		}

	}
	// if software skinning selected, use GenerateSkinnedMesh to create a mesh that can be used with UpdateSkinnedMesh
	else if (g_SkinningMethod == SOFTWARE)
	{
		hr = pMeshContainer->pOrigMesh->CloneMeshFVF(D3DXMESH_MANAGED, pMeshContainer->pOrigMesh->GetFVF(),
			pd3dDevice, &pMeshContainer->MeshData.pMesh);
		if (FAILED(hr))
			goto e_Exit;

		hr = pMeshContainer->MeshData.pMesh->GetAttributeTable(NULL, &pMeshContainer->NumAttributeGroups);
		if (FAILED(hr))
			goto e_Exit;

		delete[] pMeshContainer->pAttributeTable;
		pMeshContainer->pAttributeTable = new D3DXATTRIBUTERANGE[pMeshContainer->NumAttributeGroups];
		if (pMeshContainer->pAttributeTable == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto e_Exit;
		}

		hr = pMeshContainer->MeshData.pMesh->GetAttributeTable(pMeshContainer->pAttributeTable, NULL);
		if (FAILED(hr))
			goto e_Exit;

		// allocate a buffer for bone matrices, but only if another mesh has not allocated one of the same size or larger
		if (g_NumBoneMatricesMax < pMeshContainer->pSkinInfo->GetNumBones())
		{
			g_NumBoneMatricesMax = pMeshContainer->pSkinInfo->GetNumBones();

			// Allocate space for blend matrices
			delete[] g_pBoneMatrices;
			g_pBoneMatrices = new D3DXMATRIXA16[g_NumBoneMatricesMax];
			if (g_pBoneMatrices == NULL)
			{
				hr = E_OUTOFMEMORY;
				goto e_Exit;
			}
		}
	}
	else  // invalid g_SkinningMethod value
	{
		// return failure due to invalid skinning method value
		hr = E_INVALIDARG;
		goto e_Exit;
	}

e_Exit:
	return hr;
}

void CModel::Draw(IDirect3DDevice9* pd3dDevice, float timeDelta)
{
	D3DXMatrixMultiply(&worldMatrix, &rotateMatrix, &worldMatrix);
	pd3dDevice->SetTransform(D3DTS_WORLD, &worldMatrix);

	if (m_pAnimController != NULL)
		m_pAnimController->AdvanceTime(timeDelta, NULL);
	UpdateFrameMatrices(m_pFrameRoot, &worldMatrix);

	DrawFrame(pd3dDevice, m_pFrameRoot);
}

void CModel::setScale(D3DXVECTOR3 setValue)
{
	D3DXMatrixScaling(&scaleMatrix, setValue.x, setValue.y, setValue.z);
	D3DXMatrixMultiply(&worldMatrix, &scaleMatrix, &worldMatrix);
}

void CModel::setMeshData(LPD3DXFRAME frameRoot, LPD3DXANIMATIONCONTROLLER animController)
{
	m_pFrameRoot = frameRoot;
	animController->CloneAnimationController(animController->GetMaxNumAnimationOutputs(), animController->GetMaxNumAnimationSets(), 
		animController->GetMaxNumTracks(), animController->GetMaxNumEvents(), &m_pAnimController);

	SetupBoneMatrixPointers(m_pFrameRoot);
}

void CModel::initModel()
{
	D3DXMatrixIdentity(&worldMatrix);
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixIdentity(&scaleMatrix);
	mCurrAnimNum = 0;
	mAnimTime = 0.0f;
}