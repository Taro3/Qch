/**
 * @file    bbsmenuloaderbase.cpp
 *
 * @brief   掲示板メニューローダ基底クラス実装
 *
 * @author  Taro.Yamada
 *
 * @date    2019/09/13
 */

#include "bbsmenuloaderbase.h"

namespace bbsmenu {

/**
 * @brief       BbsMenuLoaderBase::BbsMenuLoaderBase
 *              コンストラクタ
 * @param[in]   parent  親オブジェクトポインタ
 * @return      なし
 */
BbsMenuLoaderBase::BbsMenuLoaderBase(QObject *parent) : QObject{parent} {
}

/**
 * @brief   bbsmenu::BbsMenuLoaderBase::~BbsMenuLoaderBase
 *          デストラクタ
 * @return  なし
 */
bbsmenu::BbsMenuLoaderBase::~BbsMenuLoaderBase() {
}

}   // end of namespace
