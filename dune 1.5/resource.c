#include "dune.h"
#include <stdio.h>

void display_resource(RESOURCE resource)
{
    char res[80];
    sprintf_s(res, sizeof res, "spice = %d/%d, population = %d/%d",
        resource.spice, resource.spice_max, resource.population, resource.population_max);
    putStringXY(rectResourceMesg.Left, rectResourceMesg.Top, res);
}