#include "types.h"

typedef struct {
/*0*/ f32 value;
/*4*/ s8 interpolation;
/*5*/ s8 channel;
/*6*/ s16 timeOffset;
} AnimCurvesKeyframe;

extern f32 fn_8006BBA0(f32*, f32, f32*);
extern f32 lbl_8039AF00;
extern f32 lbl_8039AF94;

f32 fn_800CEDF0(AnimCurvesKeyframe* arg0, s32 arg1, s32 arg2) {
    s32 var_r31;
    f32 temp;
    f32 sp14[4];
    f32 var_f29;
    f32 var_f30;
    f32 var_f31;
    s32 temp_r28;
    f32 temp_f28;

    if (arg1 <= 0) {
        return lbl_8039AF00;
    }
    var_r31 = 0;
    while ((var_r31 < arg1) && (arg0[var_r31].timeOffset < arg2)) {
        var_r31 += 1;
    }
    if (var_r31 == arg1) {
        var_f30 = arg0[arg1 - 1].value;
    } else if (var_r31 == 0) {
        var_f30 = arg0->value;
    } else {
        if (arg2 == arg0[var_r31].timeOffset) {
            var_f30 = arg0[var_r31].value;
            if (((arg0[var_r31].interpolation & 3) > 1) && (var_r31 < (s32) (arg1 - 1))) {
                var_f30 = arg0[var_r31 + 1].value;
            }
            return var_f30;
        }
        var_r31 = var_r31 - 1;
        temp_r28 = arg0[var_r31].interpolation & 3;
        sp14[0] = arg0[var_r31].value;
        if (temp_r28 == 0) {
            var_f29 = arg0[var_r31 + 1].value - arg0[var_r31].value;
            if (var_r31 > 0) {
                var_f31 = arg0[var_r31].value - arg0[var_r31 - 1].value;
            } else {
                var_f31 = var_f29;
            }
            if (var_f29 < lbl_8039AF00) {
                var_f29 = -var_f29;
            }
            if (var_f31 < lbl_8039AF00) {
                var_f31 = -var_f31;
            }
            temp = (var_f29 + var_f31) / 16.0f;
            sp14[2] = temp * ((s8) arg0[var_r31].interpolation >> 2);
        }
        temp_f28 = (f32) (arg0[var_r31 + 1].timeOffset - arg0[var_r31].timeOffset);
        var_r31 = var_r31 + 1;
        if (var_r31 < arg1) {
            sp14[1] = arg0[var_r31].value;
            if (temp_r28 == 0) {
                if ((s32) (var_r31 + 1) < arg1) {
                    var_f31 = arg0[var_r31 + 1].value - arg0[var_r31].value;
                } else {
                    var_f31 = var_f29;
                }
                if (var_f31 < lbl_8039AF00) {
                    var_f31 = -var_f31;
                }
                temp = (var_f29 + var_f31) / 16.0f;
                sp14[3] = temp * (f32) ((s8) arg0[var_r31].interpolation >> 2);
            }
        }
        if (temp_f28 > lbl_8039AF00) {
            temp_f28 = (f32) (arg2 - arg0[var_r31 - 1].timeOffset) / temp_f28;
            if (temp_r28 == 0) {
                var_f30 = fn_8006BBA0(sp14, temp_f28, NULL);
            } else if (temp_r28 == 1) {
                var_f30 = (temp_f28 * (sp14[1] - sp14[0])) + sp14[0];
            } else {
                var_f30 = sp14[1];
            }
        } else {
            var_f30 = sp14[1];
        }
    }
    return var_f30;
}
