// ============================================================
// FILE: ff2019_jailbreak_all_in_one.c
// TWEAK HACK FF 2019 CHO IPHONE JAILBREAK
// MENU NỔI: thelightasean
// MODULE: AIM + HEADSHOT, TỐC ĐỘ x100, DỊCH CHUYỂN,
//         CHỐNG BAN, ĐỊNH VỊ ĐỊCH, ĐẾM ĐỊCH
// BIÊN DỊCH: Theos (make package)
// ============================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <dlfcn.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <UIKit/UIKit.h>

// ============================================================
// PHẦN 1: OFFSET BỘ NHỚ FF 2019
// ============================================================
#define OFF_NGUOI_CHOI      0x2A1B4C0
#define OFF_DS_THUC_THE     0x2A1B4D8
#define OFF_MAU             0x1F8
#define OFF_DAN             0x2C4
#define OFF_VI_TRI_X        0x120
#define OFF_VI_TRI_Y        0x124
#define OFF_VI_TRI_Z        0x128
#define OFF_DOI             0x3E0
#define OFF_XUONG_DAU       0x2B8
#define OFF_GOC_YAW         0x1A0
#define OFF_GOC_PITCH       0x1A4
#define OFF_TOC_DO          0x2E0
#define OFF_BAN             0x2C8
#define OFF_CO_ANTICHEAT    0x3F8

// ============================================================
// PHẦN 2: BIẾN TOÀN CỤC
// ============================================================
static task_t g_task = MACH_PORT_NULL;
static uint64_t g_base = 0;
static volatile int g_dang_chay = 1;
static volatile int g_ngam_dau = 1;
static volatile int g_toc_do = 1;
static volatile int g_dich_chuyen = 0;
static volatile int g_chong_ban = 1;
static volatile int g_dinh_vi = 1;
static volatile int g_dem_dich = 1;
static pthread_t g_luong[8];
static uint64_t g_nguoi_choi = 0;
static uint64_t g_muc_tieu = 0;
static UIWindow *g_menu_window = nil;
static UIButton *g_nut_hack = nil;

#define MAX_DICH 32
static float g_vi_tri_dich[MAX_DICH][3];
static int g_so_luong_dich = 0;

// ============================================================
// PHẦN 3: HÀM ĐỌC/GHI BỘ NHỚ
// ============================================================
static kern_return_t doc(uint64_t addr, void* buf, size_t sz) {
    mach_vm_size_t out;
    return mach_vm_read_overwrite(g_task, addr, sz, (mach_vm_address_t)buf, &out);
}
static kern_return_t ghi(uint64_t addr, void* buf, size_t sz) {
    return mach_vm_write(g_task, addr, (vm_offset_t)buf, (mach_msg_type_number_t)sz);
}

static uint64_t tim_base(pid_t pid) {
    mach_vm_address_t addr = 0;
    mach_vm_size_t size = 0;
    vm_region_extended_info_data_t info;
    mach_msg_type_number_t cnt = VM_REGION_EXTENDED_INFO_COUNT;
    mach_port_t obj;
    uint64_t min_addr = 0xFFFFFFFFFFFFFFFF;
    while (1) {
        kern_return_t kr = mach_vm_region(g_task, &addr, &size,
            VM_REGION_EXTENDED_INFO, (vm_region_info_t)&info, &cnt, &obj);
        if (kr != KERN_SUCCESS) break;
        if ((info.protection & VM_PROT_EXECUTE) && addr < min_addr)
            min_addr = addr;
        addr += size;
    }
    return min_addr;
}

// ============================================================
// PHẦN 4: MODULE NGẮM ĐẦU (AIM + HEADSHOT)
// ============================================================
static void* luong_ngam_dau(void* a) {
    while (g_dang_chay && g_ngam_dau) {
        if (!g_nguoi_choi) { usleep(10000); continue; }
        uint64_t ds = 0;
        doc(g_base + OFF_DS_THUC_THE, &ds, 8);
        if (!ds) { usleep(10000); continue; }
        float lx, ly, lz;
        doc(g_nguoi_choi + OFF_VI_TRI_X, &lx, 4);
        doc(g_nguoi_choi + OFF_VI_TRI_Y, &ly, 4);
        doc(g_nguoi_choi + OFF_VI_TRI_Z, &lz, 4);
        float best = 1e9f;
        uint64_t best_ent = 0;
        float tx=0, ty=0, tz=0;
        for (int i = 0; i < 32; i++) {
            uint64_t ent = 0;
            doc(ds + i*8, &ent, 8);
            if (!ent || ent == g_nguoi_choi) continue;
            int doi = 0;
            doc(ent + OFF_DOI, &doi, 4);
            if (doi == 0) continue;
            int mau = 0;
            doc(ent + OFF_MAU, &mau, 4);
            if (mau <= 0) continue;
            float hx, hy, hz;
            doc(ent + OFF_XUONG_DAU, &hx, 4);
            doc(ent + OFF_XUONG_DAU + 4, &hy, 4);
            doc(ent + OFF_XUONG_DAU + 8, &hz, 4);
            float dx = hx - lx, dy = hy - ly, dz = hz - lz;
            float d = dx*dx + dy*dy + dz*dz;
            if (d < best) { best = d; best_ent = ent; tx=hx; ty=hy; tz=hz; }
        }
        if (best_ent) {
            g_muc_tieu = best_ent;
            float dx = tx - lx, dy = ty - ly, dz = tz - lz;
            float yaw = atan2f(dy, dx) * 180.0f / M_PI;
            float hyp = sqrtf(dx*dx + dy*dy);
            float pitch = -atan2f(dz, hyp) * 180.0f / M_PI;
            ghi(g_nguoi_choi + OFF_GOC_YAW, &yaw, 4);
            ghi(g_nguoi_choi + OFF_GOC_PITCH, &pitch, 4);
            uint32_t ban = 1;
            ghi(g_nguoi_choi + OFF_BAN, &ban, 4);
        }
        usleep(3000);
    }
    return NULL;
}

// ============================================================
// PHẦN 5: MODULE TỐC ĐỘ x100
// ============================================================
static void* luong_toc_do(void* a) {
    while (g_dang_chay && g_toc_do) {
        if (g_nguoi_choi) {
            float td = 100.0f;
            ghi(g_nguoi_choi + OFF_TOC_DO, &td, 4);
            float ha = 10.0f;
            ghi(g_nguoi_choi + OFF_TOC_DO + 4, &ha, 4);
        }
        usleep(30000);
    }
    return NULL;
}

// ============================================================
// PHẦN 6: MODULE DỊCH CHUYỂN
// ============================================================
static void* luong_dich_chuyen(void* a) {
    while (g_dang_chay) {
        if (g_dich_chuyen && g_muc_tieu && g_nguoi_choi) {
            float tx, ty, tz;
            doc(g_muc_tieu + OFF_VI_TRI_X, &tx, 4);
            doc(g_muc_tieu + OFF_VI_TRI_Y, &ty, 4);
            doc(g_muc_tieu + OFF_VI_TRI_Z, &tz, 4);
            tx -= 1.5f;
            ghi(g_nguoi_choi + OFF_VI_TRI_X, &tx, 4);
            ghi(g_nguoi_choi + OFF_VI_TRI_Y, &ty, 4);
            ghi(g_nguoi_choi + OFF_VI_TRI_Z, &tz, 4);
            g_dich_chuyen = 0;
        }
        usleep(10000);
    }
    return NULL;
}

// ============================================================
// PHẦN 7: MODULE CHỐNG BAN / NÉ ANTICHEAT
// ============================================================
static void* luong_chong_ban(void* a) {
    while (g_dang_chay && g_chong_ban) {
        if (g_nguoi_choi) {
            uint32_t sach = 0;
            ghi(g_nguoi_choi + OFF_CO_ANTICHEAT, &sach, 4);
            uint64_t hash_gia = 0xDEADBEEF;
            ghi(g_nguoi_choi + OFF_CO_ANTICHEAT + 8, &hash_gia, 8);
        }
        static int nhip = 0;
        if (++nhip % 100 == 0 && g_nguoi_choi) {
            float bt = 6.0f;
            ghi(g_nguoi_choi + OFF_TOC_DO, &bt, 4);
            usleep(200000);
            float nhanh = 100.0f;
            ghi(g_nguoi_choi + OFF_TOC_DO, &nhanh, 4);
        }
        usleep(50000);
    }
    return NULL;
}

// ============================================================
// PHẦN 8: MODULE ĐỊNH VỊ ĐỊCH
// ============================================================
static void* luong_dinh_vi(void* a) {
    while (g_dang_chay && g_dinh_vi) {
        if (!g_nguoi_choi) { usleep(50000); continue; }
        uint64_t ds = 0;
        doc(g_base + OFF_DS_THUC_THE, &ds, 8);
        if (!ds) { usleep(50000); continue; }
        float lx, ly, lz;
        doc(g_nguoi_choi + OFF_VI_TRI_X, &lx, 4);
        doc(g_nguoi_choi + OFF_VI_TRI_Y, &ly, 4);
        doc(g_nguoi_choi + OFF_VI_TRI_Z, &lz, 4);
        int dem = 0;
        for (int i = 0; i < MAX_DICH; i++) {
            uint64_t ent = 0;
            doc(ds + i*8, &ent, 8);
            if (!ent || ent == g_nguoi_choi) continue;
            int doi = 0;
            doc(ent + OFF_DOI, &doi, 4);
            if (doi == 0) continue;
            int mau = 0;
            doc(ent + OFF_MAU, &mau, 4);
            if (mau <= 0) continue;
            float ex, ey, ez;
            doc(ent + OFF_VI_TRI_X, &ex, 4);
            doc(ent + OFF_VI_TRI_Y, &ey, 4);
            doc(ent + OFF_VI_TRI_Z, &ez, 4);
            g_vi_tri_dich[dem][0] = ex;
            g_vi_tri_dich[dem][1] = ey;
            g_vi_tri_dich[dem][2] = ez;
            float dx = ex - lx, dy = ey - ly, dz = ez - lz;
            float kc = sqrtf(dx*dx + dy*dy + dz*dz);
            printf("[thelightasean] Địch #%d | KC: %.1fm | X:%.1f Y:%.1f Z:%.1f\n",
                   dem+1, kc, ex, ey, ez);
            dem++;
        }
        g_so_luong_dich = dem;
        usleep(200000);
    }
    return NULL;
}

// ============================================================
// PHẦN 9: MODULE ĐẾM ĐỊCH
// ============================================================
static void* luong_dem_dich(void* a) {
    while (g_dang_chay && g_dem_dich) {
        if (!g_nguoi_choi) { usleep(100000); continue; }
        uint64_t ds = 0;
        doc(g_base + OFF_DS_THUC_THE, &ds, 8);
        if (!ds) { usleep(100000); continue; }
        int tong = 0;
        for (int i = 0; i < MAX_DICH; i++) {
            uint64_t ent = 0;
            doc(ds + i*8, &ent, 8);
            if (!ent || ent == g_nguoi_choi) continue;
            int doi = 0;
            doc(ent + OFF_DOI, &doi, 4);
            if (doi == 0) continue;
            int mau = 0;
            doc(ent + OFF_MAU, &mau, 4);
            if (mau > 0) tong++;
        }
        g_so_luong_dich = tong;
        printf("[thelightasean] Số địch còn sống: %d\n", tong);
        usleep(300000);
    }
    return NULL;
}

// ============================================================
// PHẦN 10: QUÉT NGƯỜI CHƠI
// ============================================================
static void* luong_quet(void* a) {
    while (g_dang_chay) {
        doc(g_base + OFF_NGUOI_CHOI, &g_nguoi_choi, 8);
        usleep(100000);
    }
    return NULL;
}

// ============================================================
// PHẦN 11: XỬ LÝ KÉO NÚT
// ============================================================
@interface KeoNutHandler : NSObject
@end
@implementation KeoNutHandler
+ (void)keoNut:(UIPanGestureRecognizer *)gesture {
    UIView *nut = gesture.view;
    CGPoint dich = [gesture translationInView:nut.superview];
    nut.center = CGPointMake(nut.center.x + dich.x, nut.center.y + dich.y);
    [gesture setTranslation:CGPointZero inView:nut.superview];
}
@end

// ============================================================
// PHẦN 12: MENU NỔI "thelightasean" + NÚT HACK
// ============================================================
static void hien_thi_menu_noi() {
    dispatch_async(dispatch_get_main_queue(), ^{
        UIAlertController *menu = [UIAlertController
            alertControllerWithTitle:@"thelightasean"
            message:@"Chọn chức năng"
            preferredStyle:UIAlertControllerStyleActionSheet];

        [menu addAction:[UIAlertAction actionWithTitle:
            [NSString stringWithFormat:@"NGẮM ĐẦU: %@", g_ngam_dau ? @"BẬT" : @"TẮT"]
            style:UIAlertActionStyleDefault handler:^(UIAlertAction *a) {
                g_ngam_dau = !g_ngam_dau; hien_thi_menu_noi();
            }]];

        [menu addAction:[UIAlertAction actionWithTitle:
            [NSString stringWithFormat:@"TỐC ĐỘ x100: %@", g_toc_do ? @"BẬT" : @"TẮT"]
            style:UIAlertActionStyleDefault handler:^(UIAlertAction *a) {
                g_toc_do = !g_toc_do; hien_thi_menu_noi();
            }]];

        [menu addAction:[UIAlertAction actionWithTitle:@"DỊCH CHUYỂN TỚI ĐỊCH"
            style:UIAlertActionStyleDefault handler:^(UIAlertAction *a) {
                g_dich_chuyen = 1; hien_thi_menu_noi();
            }]];

        [menu addAction:[UIAlertAction actionWithTitle:
            [NSString stringWithFormat:@"CHỐNG BAN: %@", g_chong_ban ? @"BẬT" : @"TẮT"]
            style:UIAlertActionStyleDefault handler:^(UIAlertAction *a) {
                g_chong_ban = !g_chong_ban; hien_thi_menu_noi();
            }]];

        [menu addAction:[UIAlertAction actionWithTitle:
            [NSString stringWithFormat:@"ĐỊNH VỊ ĐỊCH: %@", g_dinh_vi ? @"BẬT" : @"TẮT"]
            style:UIAlertActionStyleDefault handler:^(UIAlertAction *a) {
                g_dinh_vi = !g_dinh_vi; hien_thi_menu_noi();
            }]];

        [menu addAction:[UIAlertAction actionWithTitle:
            [NSString stringWithFormat:@"ĐẾM ĐỊCH: %d địch", g_so_luong_dich]
            style:UIAlertActionStyleDefault handler:^(UIAlertAction *a) {
                hien_thi_menu_noi();
            }]];

        [menu addAction:[UIAlertAction actionWithTitle:@"BẬT TẤT CẢ"
            style:UIAlertActionStyleDefault handler:^(UIAlertAction *a) {
                g_ngam_dau=1; g_toc_do=1; g_chong_ban=1; g_dinh_vi=1; g_dem_dich=1;
                hien_thi_menu_noi();
            }]];

        [menu addAction:[UIAlertAction actionWithTitle:@"TẮT TẤT CẢ"
            style:UIAlertActionStyleDestructive handler:^(UIAlertAction *a) {
                g_ngam_dau=0; g_toc_do=0; g_chong_ban=0; g_dinh_vi=0; g_dem_dich=0;
                hien_thi_menu_noi();
            }]];

        [menu addAction:[UIAlertAction actionWithTitle:@"ĐÓNG MENU"
            style:UIAlertActionStyleCancel handler:nil]];

        UIViewController *root = [UIApplication sharedApplication].keyWindow.rootViewController;
        [root presentViewController:menu animated:YES completion:nil];
    });
}

static void tao_nut_noi() {
    dispatch_async(dispatch_get_main_queue(), ^{
        g_menu_window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
        g_menu_window.windowLevel = UIWindowLevelAlert + 1;
        g_menu_window.backgroundColor = [UIColor clearColor];
        g_menu_window.rootViewController = [[UIViewController alloc] init];
        g_menu_window.rootViewController.view.backgroundColor = [UIColor clearColor];
        [g_menu_window makeKeyAndVisible];

        g_nut_hack = [UIButton buttonWithType:UIButtonTypeCustom];
        g_nut_hack.frame = CGRectMake(20, 100, 140, 50);
        g_nut_hack.backgroundColor = [UIColor colorWithRed:1.0 green:0.6 blue:0.0 alpha:0.9];
        g_nut_hack.layer.cornerRadius = 25;
        g_nut_hack.layer.borderWidth = 2;
        g_nut_hack.layer.borderColor = [UIColor whiteColor].CGColor;
        [g_nut_hack setTitle:@"thelightasean" forState:UIControlStateNormal];
        [g_nut_hack setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
        g_nut_hack.titleLabel.font = [UIFont boldSystemFontOfSize:13];

        [g_nut_hack addTarget:[KeoNutHandler class]
            action:@selector(keoNut:)
            forControlEvents:UIControlEventTouchUpInside];

        UIPanGestureRecognizer *keo = [[UIPanGestureRecognizer alloc]
            initWithTarget:[KeoNutHandler class] action:@selector(keoNut:)];
        [g_nut_hack addGestureRecognizer:keo];

        // Nhấn nút mở menu (dùng UITapGestureRecognizer để tránh xung đột)
        UITapGestureRecognizer *nhap = [[UITapGestureRecognizer alloc]
            initWithTarget:[KeoNutHandler class] action:@selector(moMenu:)];
        [g_nut_hack addGestureRecognizer:nhap];

        [g_menu_window.rootViewController.view addSubview:g_nut_hack];
    });
}

// Thêm phương thức mở menu
@interface KeoNutHandler (Menu)
+ (void)moMenu:(UITapGestureRecognizer *)tap;
@end
@implementation KeoNutHandler (Menu)
+ (void)moMenu:(UITapGestureRecognizer *)tap {
    hien_thi_menu_noi();
}
@end

// ============================================================
// PHẦN 13: ĐIỂM VÀO TWEAK (Theos)
// ============================================================
__attribute__((constructor))
static void khoi_tao_hack() {
    printf("[thelightasean] Đang tải tweak hack...\n");

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 3 * NSEC_PER_SEC),
                   dispatch_get_main_queue(), ^{
        tao_nut_noi();
    });

    pid_t pid = getpid();
    if (task_for_pid(mach_task_self(), pid, &g_task) != KERN_SUCCESS) {
        printf("[thelightasean] task_for_pid thất bại\n");
        return;
    }
    g_base = tim_base(pid);
    printf("[thelightasean] base = 0x%llx\n", g_base);

    pthread_create(&g_luong[0], NULL, luong_quet, NULL);
    pthread_create(&g_luong[1], NULL, luong_ngam_dau, NULL);
    pthread_create(&g_luong[2], NULL, luong_toc_do, NULL);
    pthread_create(&g_luong[3], NULL, luong_dich_chuyen, NULL);
    pthread_create(&g_luong[4], NULL, luong_chong_ban, NULL);
    pthread_create(&g_luong[5], NULL, luong_dinh_vi, NULL);
    pthread_create(&g_luong[6], NULL, luong_dem_dich, NULL);

    printf("[thelightasean] Module đã kích hoạt – nhấn nút thelightasean để mở menu\n");
}

__attribute__((destructor))
static void huy_hack() {
    g_dang_chay = 0;
    for (int i = 0; i < 7; i++) pthread_join(g_luong[i], NULL);
    if (g_task) mach_port_deallocate(mach_task_self(), g_task);
}

// ============================================================
// PHẦN 14: XUẤT HÀM ĐIỀU KHIỂN
// ============================================================
void bat_ngam_dau(int v)  { g_ngam_dau = v; }
void bat_toc_do(int v)    { g_toc_do = v; }
void kich_dich_chuyen()   { g_dich_chuyen = 1; }
void bat_chong_ban(int v) { g_chong_ban = v; }
void bat_dinh_vi(int v)   { g_dinh_vi = v; }
void bat_dem_dich(int v)  { g_dem_dich = v; }
int  lay_so_dich()        { return g_so_luong_dich; }
void bat_tat_ca()  { g_ngam_dau=1; g_toc_do=1; g_chong_ban=1; g_dinh_vi=1; g_dem_dich=1; }
void tat_tat_ca()  { g_ngam_dau=0; g_toc_do=0; g_chong_ban=0; g_dinh_vi=0; g_dem_dich=0; }
