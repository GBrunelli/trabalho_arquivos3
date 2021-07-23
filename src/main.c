#include "project.h"

#include "join.h"
#include "sort.h"

int main(void)
{
    // Checking which routine should we execute
    int n_routine = 0;
    scanf("%d", &n_routine);

    // Switching for correct task
    switch (n_routine)
    {
    case 15:
        baseJoin(nestedStrategy);
        break;
    case 16:
        baseJoin(indexedStrategy);
        break;
    case 17:
        baseSort(carStrategy);
        break;
    case 18:
        baseSort(lineStrategy);
        break;
    case 19:
        baseJoin(sortedStrategy);
        break;
    }

    return EXIT_SUCCESS;
}