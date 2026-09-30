// CharForge.cpp Created by knight-gongjian 2004.12.7.
//---------------------------------------------------------
#include "stdafx.h"
#include "CharForge.h"
#include <ForgeRecord.h>
#include "GameAppNet.h"
#include "Player.h"

//---------------------------------------------------------

mission::CForgeSystem g_ForgeSystem;

namespace mission
{

	CForgeSystem::CForgeSystem()
	{
		m_pRecordSet = NULL;
	}

	CForgeSystem::~CForgeSystem()
	{
		Clear();
	}

	void CForgeSystem::Clear()
	{
		SAFE_DELETE( m_pRecordSet );
	}

	BOOL CForgeSystem::LoadForgeData( char szName[] )
	{
		Clear();
		
		extern BOOL LoadTable(CRawDataSet *pTable, const char*);
		m_pRecordSet = new CForgeRecordSet( 1, ROLE_MAXNUM_FORGE );
		return LoadTable(m_pRecordSet, szName);
	}

	void CForgeSystem::ForgeItem( CCharacter& character, BYTE byIndex )
	{
		// 判断是否在交易状态
		if( character.m_CKitbag.IsLock() )
		{
			character.SystemNotice( "\xc4\xe3\xb5\xc4\xb1\xb3\xb0\xfc\xd2\xd1\xb1\xbb\xcb\xf8\xb6\xa8\xa3\xac\xb2\xbb\xbf\xc9\xd2\xd4\xbe\xab\xc1\xb7\xce\xef\xc6\xb7\xa3\xa1" );
			return;
		}

        //密码锁定
        if( character.m_CKitbag.IsPwdLocked() )
        {
            character.SystemNotice( "\xc4\xe3\xb5\xc4\xb1\xb3\xb0\xfc\xd2\xd1\xb1\xbb\xc3\xdc\xc2\xeb\xcb\xf8\xb6\xa8\xa3\xac\xb2\xbb\xbf\xc9\xd2\xd4\xbe\xab\xc1\xb7\xce\xef\xc6\xb7\xa3\xa1" );
			return;
        }
		//add by ALLEN 2007-10-16
				if( character.IsReadBook() )
        {
            character.SystemNotice( "\xd5\xfd\xd4\xda\xb6\xc1\xca\xe9\xa3\xac\xb2\xbb\xbf\xc9\xd2\xd4\xbe\xab\xc1\xb7\xce\xef\xc6\xb7\xa3\xa1" );
			return;
        }
		SItemGrid *pItemData;
		if( !(pItemData = character.m_CKitbag.GetGridContByID( byIndex )) )
		{
			character.SystemNotice( "ForgeItem:\xb4\xed\xce\xf3\xb5\xc4\xb1\xb3\xb0\xfc\xc0\xb8\xce\xbb\xcb\xf7\xd2\xfd\xa3\xa1 ID = %d", byIndex );
			return;
		}
		
		CItemRecord* pItem = GetItemRecordInfo( pItemData->sID );
		if( pItem == NULL )
		{
			character.SystemNotice( "ForgeItem:\xb4\xed\xce\xf3\xb5\xc4\xbe\xab\xc1\xb7\xce\xef\xc6\xb7\xa3\xa1 ID = %d", pItemData->sID );
			return;
		}

		if( pItem->chForgeLv == 0 )
		{
			character.SystemNotice( "\xce\xef\xc6\xb7\xa1\xb6%s\xa1\xb7\xb2\xbb\xbf\xc9\xd2\xd4\xbe\xab\xc1\xb7\xa3\xa1", pItem->szName );
			return;
		}

		BYTE byLevel = pItemData->chForgeLv;
		if( byLevel >= ROLE_MAXNUM_FORGE )
		{
			character.SystemNotice( "\xc4\xe3\xb5\xc4\xa1\xb6%s\xa1\xb7\xd2\xd1\xbe\xad\xca\xc7\xbe\xab\xc1\xb7\xb6\xa5\xbc\xb6\xd7\xb0\xb1\xb8\xa3\xa1", pItem->szName );
			return;
		}
		
		// 精练到下一级
		byLevel++;

		CForgeRecord* pRecord = (CForgeRecord*)m_pRecordSet->GetRawDataInfo( byLevel );
		if( !pRecord )
		{
			character.SystemNotice( "ForgeItem:\xce\xde\xd0\xa7\xb5\xc4\xbe\xab\xc1\xb7\xb5\xc8\xbc\xb6\xa3\xa1Level = %d", byLevel );
			return;
		}

		if( !character.HasMoney( pRecord->dwMoney ) )
		{
			character.SystemNotice( "\xbe\xab\xc1\xb7\xb5\xc0\xbe\xdf\xcb\xf9\xd0\xe8\xbd\xf0\xc7\xae\xb2\xbb\xd7\xe3\xa3\xac\xbe\xab\xc1\xb7\xca\xa7\xb0\xdc\xa3\xa1" );
			return;
		}

		for( int i = 0; i < FORGE_MAXNUM_ITEM; i++ )
		{
			if( pRecord->ForgeItem[i].sItem == 0 )
				break;
			if( !character.HasItem( pRecord->ForgeItem[i].sItem, pRecord->ForgeItem[i].byNum ) )
			{
				char szForgeItem[64] = "\xce\xb4\xd6\xaa";
				CItemRecord* pForgeItem = (CItemRecord*)GetItemRecordInfo( pRecord->ForgeItem[i].sItem );
				if( pForgeItem )
				{
					strcpy( szForgeItem, pForgeItem->szName );
				}
				character.SystemNotice( "\xc8\xb1\xc9\xd9\xbe\xab\xc1\xb7\xd0\xe8\xc7\xf3\xce\xef\xc6\xb7\xa1\xb6%s\xa1\xb7\xa3\xac\xb9\xb2\xbc\xc6%d\xb8\xf6\xa3\xac\xbe\xab\xc1\xb7\xca\xa7\xb0\xdc\xa3\xa1", szForgeItem, pRecord->ForgeItem[i].byNum );
				return;
			}
		}

		BOOL bSuccess = TRUE;
		// 判断是否超出
		if( byLevel > pItem->chForgeLv )
		{
			// 超过精练物品允许精练等级失败，惩罚
			bSuccess = FALSE;
		}
		else
		{
			// 判断是否 超过精练安定值
			if( byLevel > pItem->chForgeSteady )
			{
				// 
				if( rand()%100 >= pRecord->byRate )
				{
					// 精练物品运气差失败，惩罚
					bSuccess = FALSE;
				}
			}
		}

		for( int i = 0; i < FORGE_MAXNUM_ITEM; i++ )
		{
			if( pRecord->ForgeItem[i].sItem == 0 )
				break;
			if( !character.TakeItem( pRecord->ForgeItem[i].sItem, pRecord->ForgeItem[i].byNum, "\xcf\xb5\xcd\xb3" ) )
			{
				char szForgeItem[64] = "\xce\xb4\xd6\xaa";
				CItemRecord* pForgeItem = (CItemRecord*)GetItemRecordInfo( pRecord->ForgeItem[i].sItem );
				if( pForgeItem )
				{
					strcpy( szForgeItem, pForgeItem->szName );
				}
				character.SystemNotice( "\xc8\xa1\xd7\xdf\xbe\xab\xc1\xb7\xd0\xe8\xc7\xf3\xce\xef\xc6\xb7\xa1\xb6%s\xa1\xb7\xa3\xac\xb9\xb2\xbc\xc6%d\xb8\xf6\xca\xa7\xb0\xdc\xa3\xa1", szForgeItem, pRecord->ForgeItem[i].byNum );
				return;
			}
		}

		if( bSuccess )
		{
			// 设置成功后精练等级
			character.m_CKitbag.SetChangeFlag(false);
			SItemGrid* pGrid = character.m_CKitbag.GetGridContByID( byIndex );
			if( pGrid == NULL || !character.ItemForge( pGrid, byLevel ) )
			{
				character.SystemNotice( "\xb4\xed\xce\xf3\xa3\xba\xbe\xab\xc1\xb7\xb3\xc9\xb9\xa6\xa3\xac\xc9\xe8\xd6\xc3\xce\xef\xc6\xb7\xa1\xb6%s\xa1\xb7\xbe\xab\xc1\xb7\xb5\xc8\xbc\xb6(%d)\xca\xa7\xb0\xdc\xa3\xa1", pItem->szName, byLevel );
				return;
			}

			character.SystemNotice( "\xbe\xab\xc1\xb7\xce\xef\xc6\xb7\xa1\xb6%s\xa1\xb7\xb3\xc9\xb9\xa6\xa3\xac\xb5\xb1\xc7\xb0\xbe\xab\xc1\xb7\xb5\xc8\xbc\xb6(%d)\xa3\xa1", pItem->szName, byLevel );
			character.SynKitbagNew( enumSYN_KITBAG_FORGES );
		}
		else
		{
			if( pRecord->byFailure == BYTE(-1) )
			{
				character.m_CKitbag.SetChangeFlag(false);
				character.KbClearItem( true, true, byIndex );
				character.SystemNotice( "\xbe\xab\xc1\xb7\xce\xef\xc6\xb7\xa1\xb6%s\xa1\xb7\xb2\xbb\xd0\xd2\xca\xa7\xb0\xdc\xa3\xac\xb5\xbc\xd6\xc2\xce\xef\xc6\xb7\xcb\xf0\xbb\xd9\xa3\xa1", pItem->szName );
				character.SynKitbagNew( enumSYN_KITBAG_FORGEF );
			}
			else
			{
				// 设置成功后精练等级
				character.m_CKitbag.SetChangeFlag(false);
				byLevel = pRecord->byFailure;
				SItemGrid* pGrid = character.m_CKitbag.GetGridContByID( byIndex );
				if( pGrid == NULL ||  !character.ItemForge( pGrid, byLevel ) )
				{
					character.SystemNotice( "\xbe\xab\xc1\xb7\xca\xa7\xb0\xdc\xa3\xac\xc9\xe8\xd6\xc3\xce\xef\xc6\xb7\xa1\xb6%s\xa1\xb7\xbe\xab\xc1\xb7\xb5\xc8\xbc\xb6(%d)\xca\xa7\xb0\xdc\xa3\xa1", pItem->szName, byLevel );
					return;
				}

				character.SystemNotice( "\xbe\xab\xc1\xb7\xce\xef\xc6\xb7\xa1\xb6%s\xa1\xb7\xca\xa7\xb0\xdc\xa3\xac\xcd\xcb\xbb\xd8\xb5\xbd\xb5\xb1\xc7\xb0\xbe\xab\xc1\xb7\xb5\xc8\xbc\xb6(%d)\xa3\xa1", pItem->szName, byLevel );
				character.SynKitbagNew( enumSYN_KITBAG_FORGEF );
			}
		}
	}

}
