#include "ration_data.h"

#include <string.h>
#include <stdio.h>

// ============================================================
// RFID CARDS + FAMILY DATA
// ============================================================
//
// Real RFID cards currently available:
//
// 99:A4:90:54 -> Kumar Family
// C9:13:8F:54 -> Sharma Family
// 72:D7:7E:5C -> Reddy Family
//
// UID is stored WITHOUT ':' because rfid_task.cpp generates
// the UID string in this format:
// 99A49054
// C9138F54
// 72D77E5C
// ============================================================

RationCard cards[NUM_CARDS] =
{
// --------------------------------------------------------
// CARD 1 - KUMAR FAMILY
// RFID: 99:A4:90:54
// --------------------------------------------------------
{
"99A49054",
"Kumar Family",


    2,

    {
        {"Ravi Kumar", 45, true},
        {"Sita Kumar", 42, true}
    },

    0.0f,  // wheatAllotted
    0.0f,  // wheatUsed

    0.0f,  // riceAllotted
    0.0f,  // riceUsed

    0.0f,  // dalAllotted
    0.0f,  // dalUsed

    0.0f,  // sugarAllotted
    0.0f,  // sugarUsed

    0       // lastResetMonth
},

// --------------------------------------------------------
// CARD 2 - SHARMA FAMILY
// RFID: C9:13:8F:54
// --------------------------------------------------------
{
    "C9138F54",
    "Sharma Family",

    4,

    {
        {"Vikram Sharma", 50, true},
        {"Anita Sharma", 47, true},
        {"Rohan Sharma", 16, false},
        {"Priya Sharma", 12, false}
    },

    0.0f,
    0.0f,

    0.0f,
    0.0f,

    0.0f,
    0.0f,

    0.0f,
    0.0f,

    0
},

// --------------------------------------------------------
// CARD 3 - REDDY FAMILY
// RFID: 72:D7:7E:5C
// --------------------------------------------------------
{
    "72D77E5C",
    "Reddy Family",

    4,

    {
        {"Lakshmi Reddy", 38, true},
        {"Sanjay Reddy", 14, false},
        {"Divya Reddy", 11, false},
        {"Kiran Reddy", 8, false}
    },

    0.0f,
    0.0f,

    0.0f,
    0.0f,

    0.0f,
    0.0f,

    0.0f,
    0.0f,

    0
},

// --------------------------------------------------------
// CARD 4 - IYER FAMILY
// No physical RFID UID assigned yet
// --------------------------------------------------------
{
    "B3C4D5E6",
    "Iyer Family",

    6,

    {
        {"Suresh Iyer", 44, true},
        {"Meena Iyer", 40, true},
        {"Karthik Iyer", 17, false},
        {"Deepa Iyer", 15, false},
        {"Arun Iyer", 9, false},
        {"Nisha Iyer", 6, false}
    },

    0.0f,
    0.0f,

    0.0f,
    0.0f,

    0.0f,
    0.0f,

    0.0f,
    0.0f,

    0
},

// --------------------------------------------------------
// CARD 5 - SINGH FAMILY
// No physical RFID UID assigned yet
// --------------------------------------------------------
{
    "F7A8B9C0",
    "Singh Family",

    4,

    {
        {"Harpreet Singh", 55, true},
        {"Gurmeet Singh", 52, true},
        {"Amandeep Singh", 22, true},
        {"Simran Singh", 9, false}
    },

    0.0f,
    0.0f,

    0.0f,
    0.0f,

    0.0f,
    0.0f,

    0.0f,
    0.0f,

    0
}


};

// ============================================================
// CALCULATE MONTHLY RATION
// ============================================================

void computeAllotment(RationCard &card)
{
float wheat = 0.0f;
float rice  = 0.0f;
float dal   = 0.0f;
float sugar = 0.0f;


for (int i = 0; i < card.memberCount; i++)
{
    if (card.members[i].isAdult)
    {
        wheat += WHEAT_PER_ADULT;
        rice  += RICE_PER_ADULT;
        dal   += DAL_PER_ADULT;
        sugar += SUGAR_PER_ADULT;
    }
    else
    {
        wheat += WHEAT_PER_CHILD;
        rice  += RICE_PER_CHILD;
        dal   += DAL_PER_CHILD;
        sugar += SUGAR_PER_CHILD;
    }
}

// Limit each grain to the maximum allowed amount.
if (wheat > MAX_GRAIN_LIMIT_KG)
    wheat = MAX_GRAIN_LIMIT_KG;

if (rice > MAX_GRAIN_LIMIT_KG)
    rice = MAX_GRAIN_LIMIT_KG;

if (dal > MAX_GRAIN_LIMIT_KG)
    dal = MAX_GRAIN_LIMIT_KG;

if (sugar > MAX_GRAIN_LIMIT_KG)
    sugar = MAX_GRAIN_LIMIT_KG;

card.wheatAllotted = wheat;
card.riceAllotted  = rice;
card.dalAllotted   = dal;
card.sugarAllotted = sugar;


}

// ============================================================
// DEMO USAGE
// ============================================================
//
// This gives the machine some previous monthly usage so that
// the display can demonstrate "remaining ration".
//
// You can change these values later.
// ============================================================

void seedDemoUsage(RationCard &card, int index)
{
switch (index)
{
case 0:
// Kumar
card.wheatUsed = 2.0f;
card.riceUsed  = 1.0f;
card.dalUsed   = 1.0f;
card.sugarUsed = 0.5f;
break;


    case 1:
        // Sharma
        card.wheatUsed = 4.0f;
        card.riceUsed  = 3.0f;
        card.dalUsed   = 2.0f;
        card.sugarUsed = 1.0f;
        break;

    case 2:
        // Reddy
        card.wheatUsed = 3.0f;
        card.riceUsed  = 2.5f;
        card.dalUsed   = 1.0f;
        card.sugarUsed = 0.5f;
        break;

    case 3:
        // Iyer
        card.wheatUsed = 5.0f;
        card.riceUsed  = 4.0f;
        card.dalUsed   = 2.0f;
        card.sugarUsed = 1.0f;
        break;

    case 4:
        // Singh
        card.wheatUsed = 4.0f;
        card.riceUsed  = 3.0f;
        card.dalUsed   = 1.5f;
        card.sugarUsed = 1.0f;
        break;

    default:
        card.wheatUsed = 0.0f;
        card.riceUsed  = 0.0f;
        card.dalUsed   = 0.0f;
        card.sugarUsed = 0.0f;
        break;
}


}

// ============================================================
// FIND CARD FROM RFID UID
// ============================================================

int findCardByUID(const uint8_t *uid, uint8_t uidLen)
{
if (uid == nullptr || uidLen == 0)
return -1;


char uidString[32] = {};
int position = 0;

for (int i = 0; i < uidLen; i++)
{
    if (position >= (int)sizeof(uidString) - 3)
        break;

    position += snprintf(
        uidString + position,
        sizeof(uidString) - position,
        "%02X",
        uid[i]
    );
}

// Compare generated UID against stored UID strings.
for (int i = 0; i < NUM_CARDS; i++)
{
    if (strcmp(uidString, cards[i].uid) == 0)
    {
        return i;
    }
}

return -1;

}

