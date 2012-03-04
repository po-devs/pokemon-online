#ifndef BATTLEFUNCTION_H
#define BATTLEFUNCTION_H

inline int minMax(int min, int max, int gen, unsigned random) {
    if (max-min != 3) {
        return (random%(max+1-min)) + min;
    } else if (gen <= 4) {
        switch(random % 8) {
        case 0:case 1:case 2: return min;
        case 3:case 4:case 5: return min +1;
        case 6: return min+2;
        case 7: default: return max;
        }
    } else {
        switch(random % 6) {
        case 0:case 1: return min;
        case 2:case 3: return min +1;
        case 4: return min+2;
        case 5: default: return max;
        }
    }
}

#endif // BATTLEFUNCTION_H
