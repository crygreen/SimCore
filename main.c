#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <windows.h> 

#define MAX_CITIZENS 100
#define MAX_PROFESSIONS 20

// --- BUILDINGS COORDINATES ---
int homes_x[4] = {100, 300, 100, 300};
int homes_y[4] = {100, 100, 500, 500};
int works_x[2] = {700, 700};
int works_y[2] = {200, 400};
int park_x = 500;
int park_y = 300;

// --- NEW: Profession Structure ---
struct Profession {
    char name[30];
    int hourly_pay;
    int hours;
    int stress;
};

struct Citizen
{
    char name[20];
    int age;
    int profession_id; // Replaces career_id
    int money;
    int mood;
    int addiction;
    int curiosity;
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

// --- LOAD PROFESSIONS FROM FILE ---
int load_professions(struct Profession *professions) {
    FILE *file = fopen("professions.txt", "r");
    if (file == NULL) {
        printf("Warning: professions.txt not found.\n");
        return 0;
    }

    int count = 0;
    while (fscanf(file, "%29[^,], %d, %d, %d", 
                  professions[count].name, 
                  &professions[count].hourly_pay, 
                  &professions[count].hours, 
                  &professions[count].stress) == 4) {
        count++;
        if (count >= MAX_PROFESSIONS) break;
    }

    fclose(file);
    return count; 
}

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

void work(struct Citizen *citizen, struct Profession prof)
{
    if (citizen->energy < 20 || citizen->health < 30) return;

    // Calculate income based on hourly pay and hours from file
    int income = prof.hours * prof.hourly_pay;

    // Apply stress from profession
    citizen->energy -= prof.stress;
    citizen->mood -= prof.stress / 2;
    
    if (citizen->energy < 0) citizen->energy = 0;
    if (citizen->mood < 0) citizen->mood = 0;

    citizen->money += income;
    citizen->hunger += 30;
    if (citizen->hunger > 100) citizen->hunger = 100;
    citizen->sleepiness += 30;
    if (citizen->sleepiness > 100) citizen->sleepiness = 100;
}

void make_music(struct Citizen *citizen)
{
    if (citizen->energy < 30) return;
    citizen->energy -= 20;
    citizen->mood += 10; 
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

void execute_decision(struct Citizen *citizen, int decision, struct Profession *professions)
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
        work(citizen, professions[citizen->profession_id]); 
    }
    else if (decision == 3) { 
        citizen->target_x = citizen->work_x; 
        citizen->target_y = citizen->work_y;
        make_music(citizen); 
    }
}

// --- DECISION LOGIC ---
int make_decision(struct Citizen *citizen, struct Profession *professions)
{
    int weights[4] = {30, 50, 50, 10}; 
    
    // Musicians prefer making music
    if (strstr(professions[citizen->profession_id].name, "Musician") != NULL) {
        weights[3] += 50;
    } else {
        weights[3] -= 30; // Others don't care much about music
    }

    if (citizen->mood < 50) weights[0] += 50 - citizen->mood;
    if (citizen->health < 60) weights[0] += 60 - citizen->health;
    
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

struct Citizen create_random_citizen(char *name, int prof_count)
{
    struct Citizen c;
    strcpy(c.name, name);
    c.age = 18 + (rand() % 15);
    
    c.profession_id = rand() % prof_count; // Pick random profession from file

    c.money = 500 + (rand() % 1500);
    c.mood = 50 + (rand() % 50);
    c.addiction = rand() % 40;
    c.curiosity = rand() % 100;
    c.savings = 0;
    c.energy = 50 + (rand() % 50);
    c.hunger = rand() % 50;
    c.sleepiness = rand() % 50;
    c.health = 50 + (rand() % 50);
    c.music_skill = rand() % 50; 
    c.fame = 0;
    
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

void log_stats_to_csv(struct City *city, int day, struct Profession *professions)
{
    FILE *file = fopen("simcore_stats.csv", "a");
    if (file == NULL) return;
    fseek(file, 0, SEEK_END);
    if (ftell(file) == 0) {
        fprintf(file, "Day,Name,Money,Health,Mood,Energy,MusicSkill,Fame,X,Y,Profession\n");
    }
    for (int i = 0; i < city->citizen_count; i++) {
        struct Citizen *c = city->citizens[i];
        fprintf(file, "%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,%s\n",
                day, c->name, c->money, c->health, c->mood, c->energy, c->music_skill, c->fame, c->x, c->y, professions[c->profession_id].name);
    }
    fclose(file);
}

void process_ai_actions(struct City *city) {
    FILE *file = fopen("ai_actions.txt", "r");
    if (file == NULL) return; 

    char action[20], from_name[20], to_name[20];
    int amount;

    if (fscanf(file, "%s %s %s %d", action, from_name, to_name, &amount) == 4) {
        int from_idx = -1, to_idx = -1;
        for (int i = 0; i < city->citizen_count; i++) {
            if (strcmp(city->citizens[i]->name, from_name) == 0) from_idx = i;
            if (strcmp(city->citizens[i]->name, to_name) == 0) to_idx = i;
        }

        if (from_idx != -1 && to_idx != -1) {
            struct Citizen *giver = city->citizens[from_idx];
            struct Citizen *receiver = city->citizens[to_idx];

            if (strcmp(action, "DONATE") == 0 && giver->money >= amount) {
                giver->money -= amount;
                receiver->money += amount;
                giver->relationships[to_idx] += 10;
                receiver->relationships[from_idx] += 10;
                printf("\n[AI Agent]: %s donated %d rubles to %s!\n", giver->name, amount, receiver->name);
            } 
            else if (strcmp(action, "ARGUE") == 0) {
                giver->mood -= 10; if(giver->mood < 0) giver->mood = 0;
                receiver->mood -= 10; if(receiver->mood < 0) receiver->mood = 0;
                giver->relationships[to_idx] -= 15;
                receiver->relationships[from_idx] -= 15;
                printf("\n[AI Agent]: %s and %s had a big argument!\n", giver->name, receiver->name);
            }
        }
    }
    fclose(file);
    remove("ai_actions.txt"); 
}

void simulate_day(struct City *city, int day, int is_auto, struct Profession *professions, int prof_count)
{
    if (!is_auto) printf("\n\n========== DAY %d ==========\n", day);
    
    process_ai_actions(city);
    
    int actions[MAX_CITIZENS]; 

    for (int i = 0; i < city->citizen_count; i++) {
        struct Citizen *citizen = city->citizens[i];
        int decision = make_decision(citizen, professions);
        actions[i] = decision;
        if (!is_auto) printf("\n%s (%s) decision: %s\n", citizen->name, professions[citizen->profession_id].name, decision_name(decision));
        execute_decision(citizen, decision, professions);
        buy_food(citizen, city);
        update_health(citizen);
    }

    for (int i = 0; i < city->citizen_count; i++) {
        struct Citizen *citizen = city->citizens[i];
        citizen->energy += 40; if (citizen->energy > 100) citizen->energy = 100;
        citizen->sleepiness = 0;
        trigger_random_event(citizen);
        if (day % 30 == 1) pay_rent(citizen, city);
        update_movement(citizen);
    }
    log_stats_to_csv(city, day, professions);
}

int main(int argc, char *argv[])
{
    SetConsoleOutputCP(CP_UTF8);
    srand((unsigned int)time(NULL));

    struct Profession professions[MAX_PROFESSIONS];
    int prof_count = load_professions(professions);
    if (prof_count == 0) {
        printf("CRITICAL ERROR: No professions loaded. Exiting.\n");
        return 1;
    }
    printf("Loaded %d professions.\n", prof_count);

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
    remove("simcore_stats.csv"); 

    // Create starting citizens
    struct Citizen genry = {"Genry", 20, 0, 1000, 70, 30, 50, 0, 50, 50, 50, 85, 10, 0, {0}, 100, 100, 100, 100, 100, 100, 700, 200};
    struct Citizen Alex = {"Alex", 25, 1, 1500, 80, 20, 60, 0, 20, 60, 40, 55, 20, 0, {0}, 300, 100, 300, 100, 300, 100, 700, 400};
    struct Citizen Mia = {"Mia", 19, 2, 700, 90, 10, 80, 0, 35, 80, 10, 100, 80, 5, {0}, 100, 500, 100, 500, 100, 500, 700, 200};

    add_citizen(&palaroid, &genry);
    add_citizen(&palaroid, &Alex);
    add_citizen(&palaroid, &Mia);

    char *new_names[10] = {"Ivan", "Elena", "Max", "Sofia", "Dmitry", "Anna", "Pavel", "Ksenia", "Artem", "Vika"};
    for (int i = 0; i < 10; i++) {
        struct Citizen *ptr = malloc(sizeof(struct Citizen));
        *ptr = create_random_citizen(new_names[i], prof_count); 
        add_citizen(&palaroid, ptr);
    }

    if (auto_days > 0) {
        printf("Auto-simulating %d days...\n", auto_days);
        for (int d = 1; d <= auto_days; d++) {
            simulate_day(&palaroid, d, 1, professions, prof_count);
            current_day++;
        }
        printf("Done! Data saved to simcore_stats.csv.\n");
    } else {
        while (1) {
            system("cls");
            simulate_day(&palaroid, current_day, 0, professions, prof_count);
            printf("\n[Day %d finished. Press ENTER for Day %d]\n", current_day, current_day + 1);
            getchar(); 
            current_day++; 
        }
    }
    return 0;
}