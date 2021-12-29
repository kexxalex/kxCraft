/*
 * Item.h
 *
 *  Created on: Dec 25, 2021
 *      Author: kexx
 */

#pragma once

struct st_drop {
    unsigned short ID;
    unsigned short count;
    st_drop *next{ nullptr };
};
