#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <windows.h> 

#define MAX_CITIZENS 100

// --- BUILDINGS COORDINATES (Must match HTML) ---
int homes_x[4] = {100, 300, 100, 300};
int homes_y[4] = {100, 100, 500, 500};
int works_x[2] = {700, 700};
int works_y[2] = {200, 400};
int park_x = 500;
int park_y = 300;

// Career names for printing
const char *CAREERS[] = {"Engineer", "Designer", "Musician", "Loader"};

struct Citizen
{
    char name[20];
    int age;
    int career_id; // 0=Engineer, 1=Designer, 2=Musician, 3=Loader
    int money;
    int mood;
    int addiction;
    int curiosity;
    int salary;
    int savings;
    int energy;
    int hunger;
    int sleepiness;
    int health;
    int music_skill;
    int fame;
    int relationships[MAX_CITIZENS];
    
    int x, y, target_x, target_y, home_x, home_y, work_x, work_y;
};

struct City
{
    char name[20];
    struct Citizen **citizens;
    int citizen_count;
    int capacity;
    int food_price;
    int rent_price;
};

// --- ACTIONS ---

void rest(struct Citizen *citizen)
{
    citizen->energy += 30;
    if (citizen->energy > 100) citizen->energy = 100;
    citizen->sleepiness -= 30;
    if (citizen->sleepiness < 0) citizen->sleepiness = 0;
    citizen->mood += 5;
    if (citizen->mood > 100) citizen->mood = 100;
}

void work(struct Citizen *citizen, int hours)
{
    if (citizen->energy < 20 || citizen->health < 30) return;

    // Salary depends on career
    int income = hours * citizen->salary / 160;
    
    // Loaders lose more energy and mood, but get paid a bit more per hour
    if (citizen->career_id == 3) {
        citizen->energy -= hours * 8;
        citizen->mood -= hours * 7;
        income = income * 120 / 100; 
    } else {
        citizen->energy -= hours * 5;
        citizen->mood -= hours * 5;
    }

    if (citizen->health < 60) income = income * 70 / 100;

    citizen->money += income;
    citizen->hunger += hours * 5;
    if (citizen->hunger > 100) citizen->hunger = 100;
    citizen->sleepiness += hours * 5;
    if (citizen->sleepiness > 100) citizen->sleepiness = 100;
    if (citizen->energy < 0) citizen->energy = 0;
    if (citizen->mood < 0) citizen->mood = 0;
}

void make_music(struct Citizen *citizen)
{
    if (citizen->energy < 30) return;
    citizen->energy -= 20;
    citizen->mood += (citizen->career_id == 2) ? 20 : 5; // Musicians get more joy
    if (citizen->mood > 100) citizen->mood = 100;

    int skill_gain = (rand() % 5) + 1;
    citizen->music_skill += skill_gain;
    if (citizen->music_skill > 100) citizen->music_skill = 100;

    if (citizen->music_skill > 50 && (rand() % 100 < 30)) {
        int fame_gain = (rand() % 3) + 1;
        citizen->fame += fame_gain;
        int income = (citizen->fame * citizen->music_skill) / 5;
        citizen->money += income;
    }
}

void execute_decision(struct Citizen *citizen, int decision)
{
    if (decision == 0) { 
        citizen->target_x = citizen->home_x; 
        citizen->target_y = citizen->home_y;
        rest(citizen); 
    }
    else if (decision == 1) { 
        citizen->target_x = park_x + (rand() % 40 - 20); 
        citizen->target_y = park_y + (rand() % 40 - 20);
        citizen->mood += 5; 
        if (citizen->mood > 100) citizen->mood = 100; 
    }
    else if (decision == 2) { 
        citizen->target_x = citizen->work_x; 
        citizen->target_y = citizen->work_y;
        work(citizen, 8); 
    }
    else if (decision == 3) { 
        citizen->target_x = citizen->work_x; 
        citizen->target_y = citizen->work_y;
        make_music(citizen); 
    }
}

int make_decision(struct Citizen *citizen)
{
    int weights[4] = {30, 50, 50, 10}; 
    weights[1] += citizen->curiosity;
    weights[3] += citizen->music_skill / 2; 

    // Musicians prefer making music, Loaders prefer staying home
    if (citizen->career_id == 2) weights[3] += 30;
    if (citizen->career_id == 3) weights[0] += 20;

    if (citizen->mood < 50) weights[0] += 50 - citizen->mood;
    weights[2] -= citizen->addiction;
    weights[0] += citizen->addiction / 2;

    if (citizen->health < 60) { weights[0] += 60 - citizen->health; weights[2] -= 60 - citizen->health; }
    for (int i = 0; i < 4; i++) { if (weights[i] < 0) weights[i] = 0; }

    int total_weight = weights[0] + weights[1] + weights[2] + weights[3];
    if (total_weight == 0) return 0;

    int random_value = rand() % total_weight;
    int accumulated = 0;
    for (int i = 0; i < 4; i++)
    {
        accumulated += weights[i];
        if (random_value < accumulated) return i;
    }
    return 0;
}

const char *decision_name(int decision)
{
    if (decision == 0) return "Rest";
    if (decision == 1) return "Walk";
    if (decision == 2) return "Work";
    if (decision == 3) return "Music";
    return "Unknown";
}

// --- CITY MANAGEMENT ---

void add_citizen(struct City *city, struct Citizen *new_citizen)
{
    if (city->citizen_count >= city->capacity)
    {
        city->capacity *= 2;
        city->citizens = realloc(city->citizens, city->capacity * sizeof(struct Citizen*));
        if (city->citizens == NULL) exit(1);
    }
    city->citizens[city->citizen_count++] = new_citizen;
}

struct Citizen create_random_citizen(char *name)
{
    struct Citizen c;
    strcpy(c.name, name);
    c.age = 18 + (rand() % 15);
    
    // Random Career (0 to 3)
    c.career_id = rand() % 4;
    
    // Base salary depends on career
    if (c.career_id == 0) c.salary = 5000; // Engineer
    else if (c.career_id == 1) c.salary = 4000; // Designer
    else if (c.career_id == 2) c.salary = 3000; // Musician
    else c.salary = 2500; // Loader

    c.money = 500 + (rand() % 1500);
    c.mood = 50 + (rand() % 50);
    c.addiction = rand() % 40;
    c.curiosity = rand() % 100;
    c.savings = 0;
    c.energy = 50 + (rand() % 50);
    c.hunger = rand() % 50;
    c.sleepiness = rand() % 50;
    c.health = 50 + (rand() % 50);
    c.music_skill = (c.career_id == 2) ? 50 + rand()%50 : rand() % 50; 
    c.fame = (c.career_id == 2) ? rand()%20 : 0;
    
    int h = rand() % 4;
    c.home_x = homes_x[h]; c.home_y = homes_y[h];
    int w = rand() % 2;
    c.work_x = works_x[w]; c.work_y = works_y[w];
    
    c.x = c.home_x; c.y = c.home_y;
    c.target_x = c.home_x; c.target_y = c.home_y;

    for (int i = 0; i < MAX_CITIZENS; i++) c.relationships[i] = 0;
    return c;
}

void update_health(struct Citizen *citizen)
{
    if (citizen->hunger > 80 || citizen->sleepiness > 80) {
        citizen->health -= 5;
        if (citizen->health < 0) citizen->health = 0;
    } else {
        citizen->health += 2;
        if (citizen->health > 100) citizen->health = 100;
    }
}

void buy_food(struct Citizen *citizen, struct City *city)
{
    if (citizen->money >= city->food_price) {
        citizen->money -= city->food_price;
        citizen->mood += 5; if (citizen->mood > 100) citizen->mood = 100;
        citizen->hunger -= 20; if (citizen->hunger < 0) citizen->hunger = 0;
    }
}

void pay_rent(struct Citizen *citizen, struct City *city)
{
    if (citizen->money >= city->rent_price) {
        citizen->money -= city->rent_price;
    } else {
        citizen->mood -= 10; if (citizen->mood < 0) citizen->mood = 0;
    }
}

void trigger_random_event(struct Citizen *citizen)
{
    if (rand() % 100 < 20) {
        int event_type = rand() % 4;
        if (event_type == 0) { citizen->money += 100; citizen->mood += 10; }
        else if (event_type == 1) { citizen->health -= 10; if(citizen->health < 0) citizen->health = 0; }
        else if (event_type == 2) { citizen->fame += 5; citizen->mood += 15; }
        else { citizen->money -= 50; citizen->mood -= 15; if(citizen->money < 0) citizen->money = 0; if(citizen->mood < 0) citizen->mood = 0; }
    }
}

void update_movement(struct Citizen *citizen)
{
    int speed = 20; 
    if (citizen->x < citizen->target_x) { citizen->x += speed; if (citizen->x > citizen->target_x) citizen->x = citizen->target_x; }
    else if (citizen->x > citizen->target_x) { citizen->x -= speed; if (citizen->x < citizen->target_x) citizen->x = citizen->target_x; }
    if (citizen->y < citizen->target_y) { citizen->y += speed; if (citizen->y > citizen->target_y) citizen->y = citizen->target_y; }
    else if (citizen->y > citizen->target_y) { citizen->y -= speed; if (citizen->y < citizen->target_y) citizen->y = citizen->target_y; }
}

void log_stats_to_csv(struct City *city, int day)
{
    FILE *file = fopen("simcore_stats.csv", "a");
    if (file == NULL) return;
    fseek(file, 0, SEEK_END);
    if (ftell(file) == 0) {
        fprintf(file, "Day,Name,Money,Health,Mood,Energy,MusicSkill,Fame,X,Y,Career\n");
    }
    for (int i = 0; i < city->citizen_count; i++) {
        struct Citizen *c = city->citizens[i];
        fprintf(file, "%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,%s\n",
                day, c->name, c->money, c->health, c->mood, c->energy, c->music_skill, c->fame, c->x, c->y, CAREERS[c->career_id]);
    }
    fclose(file);
}

void simulate_day(struct City *city, int day, int is_auto)
{
    if (!is_auto) printf("\n\n========== DAY %d ==========\n", day);
    int actions[MAX_CITIZENS]; 

    for (int i = 0; i < city->citizen_count; i++) {
        struct Citizen *citizen = city->citizens[i];
        int decision = make_decision(citizen);
        actions[i] = decision;
        execute_decision(citizen, decision);
        buy_food(citizen, city);
        update_health(citizen);
    }

    for (int i = 0; i < city->citizen_count; i++) {
        for (int j = i + 1; j < city->citizen_count; j++) {
            if (actions[i] == actions[j] && actions[i] != 0) {
                struct Citizen *c1 = city->citizens[i];
                struct Citizen *c2 = city->citizens[j];
                c1->relationships[j] += 2; c2->relationships[i] += 2;
                if (actions[i] == 3 && c1->music_skill > 50 && c2->music_skill > 50) {
                    c1->fame += 5; c2->fame += 5;
                }
            }
        }
    }

    for (int i = 0; i < city->citizen_count; i++) {
        struct Citizen *citizen = city->citizens[i];
        citizen->energy += 40; if (citizen->energy > 100) citizen->energy = 100;
        citizen->sleepiness = 0;
        trigger_random_event(citizen);
        if (day % 30 == 1) pay_rent(citizen, city);
        update_movement(citizen);
    }
    log_stats_to_csv(city, day);
}

int main(int argc, char *argv[])
{
    SetConsoleOutputCP(CP_UTF8);
    srand((unsigned int)time(NULL));

    // Check for auto-mode argument (e.g., .\main 100)
    int auto_days = 0;
    if (argc > 1) auto_days = atoi(argv[1]);

    struct City palaroid;
    strcpy(palaroid.name, "Palaroid");
    palaroid.food_price = 50;
    palaroid.rent_price = 500;
    palaroid.capacity = 3; 
    palaroid.citizen_count = 0;
    palaroid.citizens = malloc(palaroid.capacity * sizeof(struct Citizen*));

    int current_day = 1; 
    
    // Remove old CSV to start fresh
    remove("simcore_stats.csv"); 

    // Create starting citizens (fixed coordinates and careers)
    struct Citizen genry = {"Genry", 20, 0, 1000, 70, 30, 50, 5000, 0, 50, 50, 50, 85, 10, 0, {0}, 100, 100, 100, 100, 100, 100, 700, 200};
    struct Citizen Alex = {"Alex", 25, 1, 1500, 80, 20, 60, 4000, 0, 20, 60, 40, 55, 20, 0, {0}, 300, 100, 300, 100, 300, 100, 700, 400};
    struct Citizen Mia = {"Mia", 19, 2, 700, 90, 10, 80, 3000, 40000, 35, 80, 10, 100, 80, 5, {0}, 100, 500, 100, 500, 100, 500, 700, 200};

    add_citizen(&palaroid, &genry);
    add_citizen(&palaroid, &Alex);
    add_citizen(&palaroid, &Mia);

    char *new_names[10] = {"Ivan", "Elena", "Max", "Sofia", "Dmitry", "Anna", "Pavel", "Ksenia", "Artem", "Vika"};
    for (int i = 0; i < 10; i++) {
        struct Citizen *ptr = malloc(sizeof(struct Citizen));
        *ptr = create_random_citizen(new_names[i]); 
        add_citizen(&palaroid, ptr);
    }

    if (auto_days > 0) {
        printf("Auto-simulating %d days...\n", auto_days);
        for (int d = 1; d <= auto_days; d++) {
            simulate_day(&palaroid, d, 1);
            current_day++;
        }
        printf("Done! Data saved to simcore_stats.csv.\n");
    } else {
        while (1) {
            system("cls");
            simulate_day(&palaroid, current_day, 0);
            printf("\n[Day %d finished. Press ENTER for Day %d]\n", current_day, current_day + 1);
            getchar(); 
            current_day++; 
        }
    }
    return 0;
}