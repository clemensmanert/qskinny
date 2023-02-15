/******************************************************************************
 * Copyright (C) 2021 Edelhirsch Software GmbH
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#pragma once

#include <QskPushButton.h>
#include <Qt>

class RoundButton : public QskPushButton
{
    Q_OBJECT

  public:
    QSK_SUBCONTROLS( Panel )
    QSK_STATES( Top )

    RoundButton( Qt::Edge, QQuickItem* parent );
};
