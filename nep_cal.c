#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define BLUE    "\033[34m"
#define RESET   "\033[0m"

typedef struct {
    int year;
    int month; // 1-12
    int day;
} NepDate;

// Lookup table for BS 2070-2090 (21 years)
// Index 0 = 2070 BS, Index 1 = 2071 BS, ... Index 20 = 2090 BS
int nepali_month_days[21][12] = {
    {31,31,31,32,31,31,29,30,30,29,30,30},  // 2070 (Index 0)
    {31,31,32,31,31,31,30,29,30,29,30,30},  // 2071 (Index 1)
    {31,32,31,32,31,30,30,29,30,29,30,30},  // 2072 (Index 2)
    {31,32,31,32,31,30,30,30,29,29,30,31},  // 2073 (Index 3)
    {31,31,31,32,31,31,29,30,30,29,30,30},  // 2074 (Index 4)
    {31,31,32,31,31,31,30,29,30,29,30,30},  // 2075 (Index 5)
    {31,32,31,32,31,30,30,30,29,29,30,30},  // 2076 (Index 6)
    {31,32,31,32,31,30,30,30,29,29,30,31},  // 2077 (Index 7)
    {31,31,31,32,31,31,30,29,30,29,30,30},  // 2078 (Index 8)
    {31,31,32,31,31,31,30,29,30,29,30,30},  // 2079 (Index 9)
    {31,32,31,32,31,30,30,30,29,29,30,30},  // 2080 (Index 10) <- Reference year
    {31,32,31,32,31,30,30,30,29,30,29,31},  // 2081 (Index 11)
    {31,31,32,31,31,31,30,29,30,29,30,30},  // 2082 (Index 12)
    {31,31,32,31,31,31,30,29,30,29,30,30},  // 2083 (Index 13)
    {31,32,31,32,31,30,30,29,30,29,30,30},  // 2084 (Index 14)
    {31,32,31,32,31,30,30,30,29,29,30,31},  // 2085 (Index 15)
    {31,31,31,32,31,31,30,29,30,29,30,31},  // 2086 (Index 16)
    {31,31,32,31,31,31,30,29,30,29,30,30},  // 2087 (Index 17)
    {31,31,32,32,31,30,30,29,30,29,30,30},  // 2088 (Index 18)
    {31,32,31,32,31,30,30,30,29,29,30,31},  // 2089 (Index 19)
    {31,31,31,32,31,31,30,29,30,29,30,31}   // 2090 (Index 20)
};

char *nepali_month_names[12] = {
    "Baisakh","Jestha","Ashad","Shrawan","Bhadra","Ashwin",
    "Kartik","Mangsir","Poush","Magh","Falgun","Chaitra"
};

// Reference date: 2080/1/1 BS = 2023/4/14 AD (Friday)
int ref_ad_year = 2023, ref_ad_month = 4, ref_ad_day = 14;
int ref_bs_year = 2080, ref_bs_month = 1, ref_bs_day = 1;

// Function declarations
int is_leap(int year);
int ad_month_days(int month, int year);
int days_difference(int y, int m, int d);
NepDate ad_to_bs(int y, int m, int d);
int bs_weekday(int bs_y, int bs_m, int bs_d);
void print_month_ui(int bs_y, int bs_m, int highlight_day);
void print_full_year(int bs_y);

int main(int argc, char *argv[]){
    int year = -1, month = -1, day = -1;

    // Parse arguments
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-y") == 0 && i + 1 < argc) year = atoi(argv[++i]);
        else if(strcmp(argv[i], "-m") == 0 && i + 1 < argc) month = atoi(argv[++i]);
        else if(strcmp(argv[i], "-d") == 0 && i + 1 < argc) day = atoi(argv[++i]);
    }

    // Validate year
    if(year != -1 && (year < 2070 || year > 2090)){
        printf("Year out of range (2070-2090 BS)\n");
        return 1;
    }

    if(year != -1){
        if(month != -1){ // Month specified
            if(month < 1 || month > 12){
                printf("Invalid month (1-12)\n");
                return 1;
            }
            int y_index = year - 2070; // Base year is now 2070
            int days_in_month = nepali_month_days[y_index][month - 1];
            if(day != -1){ // Day specified
                if(day < 1 || day > days_in_month){
                    printf("Invalid day for %s %d BS\n", nepali_month_names[month - 1], year);
                    return 1;
                }
                printf("Nepali Date: %d-%02d-%02d BS\n", year, month, day);
                printf("Nepali Month: %s\n", nepali_month_names[month - 1]);
                print_month_ui(year, month, day);
            } else { // Only month
                print_month_ui(year, month, -1);
            }
        } else { // Only year
            print_full_year(year);
        }
        return 0;
    }

    // Default: today
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    NepDate nd = ad_to_bs(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    
    printf("Today (AD: %d-%02d-%02d) -> Nepali Date: %d-%02d-%02d BS\n",
           tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
           nd.year, nd.month, nd.day);
    printf("Nepali Month: %s\n", nepali_month_names[nd.month - 1]);
    print_month_ui(nd.year, nd.month, nd.day);

    return 0;
}

// ---------------- Helper functions -----------------

int is_leap(int year){
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int ad_month_days(int month, int year){
    int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if(month == 2 && is_leap(year)) return 29;
    return days[month - 1];
}

int days_difference(int y, int m, int d){
    struct tm ref = {0}, target = {0};
    ref.tm_year = ref_ad_year - 1900; 
    ref.tm_mon = ref_ad_month - 1; 
    ref.tm_mday = ref_ad_day;
    target.tm_year = y - 1900; 
    target.tm_mon = m - 1; 
    target.tm_mday = d;
    time_t t_ref = mktime(&ref);
    time_t t_target = mktime(&target);
    return (int)((t_target - t_ref) / (24 * 3600));
}

NepDate ad_to_bs(int y, int m, int d){
    int diff = days_difference(y, m, d);
    int bs_y = ref_bs_year, bs_m = ref_bs_month, bs_d = ref_bs_day;
    
    if(diff > 0){
        // Forward conversion
        while(diff > 0){
            int y_index = bs_y - 2070; // Base year is now 2070
            if(y_index < 0 || y_index >= 21) break; // Check against 21 years (2070-2090)
            
            int days_in_month = nepali_month_days[y_index][bs_m - 1];
            int days_remaining_in_month = days_in_month - bs_d + 1;
            
            if(diff >= days_remaining_in_month){
                diff -= days_remaining_in_month;
                bs_d = 1;
                bs_m++;
                if(bs_m > 12){
                    bs_m = 1;
                    bs_y++;
                }
            } else {
                bs_d += diff;
                diff = 0;
            }
        }
    } else if(diff < 0){
        // Backward conversion
        diff = -diff;
        while(diff > 0){
            bs_d--;
            if(bs_d <= 0){
                bs_m--;
                if(bs_m <= 0){
                    bs_m = 12;
                    bs_y--;
                }
                int y_index = bs_y - 2070;
                if(y_index < 0 || y_index >= 21) break;
                bs_d = nepali_month_days[y_index][bs_m - 1];
            }
            diff--;
        }
    }
    
    NepDate nd = {bs_y, bs_m, bs_d};
    return nd;
}

int bs_weekday(int bs_y, int bs_m, int bs_d){
    // Calculate days from reference point
    int days = 0;
    
    // Add days for complete years
    for(int y = ref_bs_year; y < bs_y; y++){
        int y_index = y - 2070;
        if(y_index < 0 || y_index >= 21) continue;
        for(int m = 0; m < 12; m++) {
            days += nepali_month_days[y_index][m];
        }
    }
    
    // Subtract days for years before reference year
    for(int y = bs_y; y < ref_bs_year; y++){
        int y_index = y - 2070;
        if(y_index < 0 || y_index >= 21) continue;
        for(int m = 0; m < 12; m++) {
            days -= nepali_month_days[y_index][m];
        }
    }
    
    // Add days for complete months in target year
    int y_index = bs_y - 2070;
    if(y_index >= 0 && y_index < 21){
        for(int m = 0; m < bs_m - 1; m++) {
            days += nepali_month_days[y_index][m];
        }
    }
    
    // Add remaining days
    days += bs_d - ref_bs_day;

    // Get weekday from reference
    struct tm ref = {0};
    ref.tm_year = ref_ad_year - 1900; 
    ref.tm_mon = ref_ad_month - 1; 
    ref.tm_mday = ref_ad_day;
    time_t t_ref = mktime(&ref);
    time_t t_target = t_ref + days * 24 * 3600;
    struct tm *tm = localtime(&t_target);
    return tm->tm_wday;
}

void print_month_ui(int bs_y, int bs_m, int highlight_day){
    int y_index = bs_y - 2070; // Base year is now 2070
    if(y_index < 0 || y_index >= 21) return; // Check against 21 years

    int days = nepali_month_days[y_index][bs_m - 1];
    printf("\n  %s %d BS\n", nepali_month_names[bs_m - 1], bs_y);
    printf("Su Mo Tu We Th Fr Sa\n");

    int first_day = bs_weekday(bs_y, bs_m, 1);
    for(int i = 0; i < first_day; i++) printf("   ");

    for(int d = 1; d <= days; d++){
        if(d == highlight_day) printf(BLUE "%2d " RESET, d);
        else printf("%2d ", d);
        if((d + first_day) % 7 == 0) printf("\n");
    }
    if((days + first_day) % 7 != 0) printf("\n"); // Add newline if needed
    printf("\n");
}

void print_full_year(int bs_y){
    printf("\n%s%d BS Calendar%s\n", GREEN, bs_y, RESET);
    printf("================\n");
    for(int m = 1; m <= 12; m++){
        print_month_ui(bs_y, m, -1);
    }
}