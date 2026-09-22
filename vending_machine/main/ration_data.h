#pragma once

#include <stdint.h>

#define NUM_CARDS 5
#define MAX_FAMILY_MEMBERS 6
#define MAX_GRAIN_LIMIT_KG 40.0f

// Monthly allotment per person.
#define WHEAT_PER_ADULT 5.0f
#define WHEAT_PER_CHILD 3.0f
#define RICE_PER_ADULT  4.0f
#define RICE_PER_CHILD  2.5f
#define DAL_PER_ADULT   2.0f
#define DAL_PER_CHILD   1.0f
#define SUGAR_PER_ADULT 1.0f
#define SUGAR_PER_CHILD 0.5f

struct FamilyMember {
    const char *name;
    uint8_t age;
    bool isAdult;
};

struct RationCard {
    const char *uid;
    const char *familyName;
    uint8_t memberCount;
    FamilyMember members[MAX_FAMILY_MEMBERS];

    float wheatAllotted;
    float wheatUsed;
    float riceAllotted;
    float riceUsed;
    float dalAllotted;
    float dalUsed;
    float sugarAllotted;
    float sugarUsed;

    int lastResetMonth;
};

extern RationCard cards[NUM_CARDS];

void computeAllotment(RationCard &card);
void seedDemoUsage(RationCard &card, int index);
int findCardByUID(const uint8_t *uid, uint8_t uidLen);
