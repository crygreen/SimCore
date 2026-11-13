#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_CITIZENS 100
#define DAYS_TO_SIMULATE 30

struct Citizen
{
    char name[20];
    int age;
    int money;
    char job[20];
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

// --- БАЗОВЫЕ ФУНКЦИИ ---

void citizen_status(struct Citizen citizen)
{
    printf("\n%s is %d years old.\n", citizen.name, citizen.age);
    printf("Job: %s | Money: %d | Mood: %d | Energy: %d | Health: %d\n", citizen.job, citizen.money, citizen.mood, citizen.energy, citizen.health);
    printf("Music Skill: %d | Fame: %d\n", citizen.music_skill, citizen.fame);
}

void work(struct Citizen *citizen, int hours)
{
    if (citizen->energy < 20) { printf("\n%s is too tired to work.\n", citizen->name); return; }
    if (citizen->health < 30) { printf("\n%s is too unhealthy to work.\n", citizen->name); return; }

    int income = hours * citizen->salary / 160;
    if (citizen->health < 60) income = income * 70 / 100;

    citizen->money += income * 90 / 100;
    citizen->savings += income * 10 / 100;
    citizen->hunger += hours * 5;
    if (citizen->hunger > 100) citizen->hunger = 100;
    citizen->sleepiness += hours * 5;
    if (citizen->sleepiness > 100) citizen->sleepiness = 100;
    citizen->energy -= hours * 5;
    if (citizen->energy < 0) citizen->energy = 0;
    citizen->mood -= hours * 5;
    if (citizen->mood < 0) citizen->mood = 0;

    printf("\n%s worked for %d hours. Earned: %d\n", citizen->name, hours, income);
}

int spend_money(struct Citizen *citizen, int amount)
{
    if (citizen->money >= amount) { citizen->money -= amount; return 1; }
    return 0;
}

void buy_food(struct Citizen *citizen, struct City *city)
{
    if (spend_money(citizen, city->food_price))
    {
        citizen->mood += 5;
        if (citizen->mood > 100) citizen->mood = 100;
        citizen->hunger -= 20;
        if (citizen->hunger < 0) citizen->hunger = 0;
        printf("\n%s bought food for %d.\n", citizen->name, city->food_price);
    }
    else { printf("\n%s cannot afford food.\n", citizen->name); }
}

void pay_rent(struct Citizen *citizen, struct City *city)
{
    if (spend_money(citizen, city->rent_price))
    {
        printf("\n%s paid rent: %d.\n", citizen->name, city->rent_price);
    }
    else
    {
        printf("\n%s cannot afford rent.\n", citizen->name);
        citizen->mood -= 10;
        if (citizen->mood < 0) citizen->mood = 0;
    }
}

void city_status(struct City city)
{
    printf("\n====================\n");
    printf("City: %s | Food: %d | Rent: %d | Citizens: %d\n", city.name, city.food_price, city.rent_price, city.citizen_count);
    printf("====================\n");
}

void rest(struct Citizen *citizen)
{
    citizen->energy += 30;
    if (citizen->energy > 100) citizen->energy = 100;
    citizen->sleepiness -= 30;
    if (citizen->sleepiness < 0) citizen->sleepiness = 0;
    citizen->mood += 5;
    if (citizen->mood > 100) citizen->mood = 100;
    printf("\n%s rested.\n", citizen->name);
}

void update_health(struct Citizen *citizen)
{
    if (citizen->hunger > 80 || citizen->sleepiness > 80)
    {
        citizen->health -= 5;
        if (citizen->health < 0) citizen->health = 0;
        printf("\n%s's health decreased.\n", citizen->name);
    }
    else
    {
        citizen->health += 2;
        if (citizen->health > 100) citizen->health = 100;
        printf("\n%s's health improved.\n", citizen->name);
    }
}

void make_music(struct Citizen *citizen)
{
    if (citizen->energy < 30) { printf("\n%s is too tired to make music.\n", citizen->name); return; }

    citizen->energy -= 20;
    citizen->mood += 10;
    if (citizen->mood > 100) citizen->mood = 100;

    int skill_gain = (rand() % 5) + 1;
    citizen->music_skill += skill_gain;
    if (citizen->music_skill > 100) citizen->music_skill = 100;

    printf("\n%s spent the day making music. Skill increased by %d.\n", citizen->name, skill_gain);

    if (citizen->music_skill > 50)
    {
        int fame_gain = (rand() % 3) + 1;
        citizen->fame += fame_gain;
        if (citizen->fame > 100) citizen->fame = 100;
        int income = (citizen->fame * citizen->music_skill) / 5;
        if (income > 0)
        {
            citizen->money += income;
            printf("The track got attention! Gained %d fame and earned %d.\n", fame_gain, income);
        }
    }
}

// --- ЛОГИКА ПРИНЯТИЯ РЕШЕНИЙ ---

int make_decision(struct Citizen *citizen)
{
    int weights[4] = {30, 50, 50, 10}; // 0=home, 1=outside, 2=work, 3=music
    weights[1] += citizen->curiosity;
    weights[3] += citizen->music_skill / 2; 

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
    if (decision == 0) return "Stay home";
    if (decision == 1) return "Go outside";
    if (decision == 2) return "Work";
    if (decision == 3) return "Make music";
    return "Unknown";
}

void execute_decision(struct Citizen *citizen, int decision)
{
    if (decision == 0) { printf("\n%s stayed at home.\n", citizen->name); rest(citizen); }
    else if (decision == 1) { printf("\n%s went outside.\n", citizen->name); citizen->mood += 5; if (citizen->mood > 100) citizen->mood = 100; }
    else if (decision == 2) { printf("\n%s decided to work.\n", citizen->name); work(citizen, 8); }
    else if (decision == 3) { make_music(citizen); }
}

// --- ДИНАМИЧЕСКАЯ ПАМЯТЬ И СОЦИАЛКА ---

void add_citizen(struct City *city, struct Citizen *new_citizen)
{
    if (city->citizen_count >= city->capacity)
    {
        city->capacity = city->capacity * 2;
        city->citizens = realloc(city->citizens, city->capacity * sizeof(struct Citizen*));
        if (city->citizens == NULL) { printf("Memory error!\n"); exit(1); }
        printf("[System]: City expanded to %d slots.\n", city->capacity);
    }
    city->citizens[city->citizen_count] = new_citizen;
    city->citizen_count++;
}

struct Citizen create_random_citizen(char *name)
{
    struct Citizen c;
    strcpy(c.name, name);
    c.age = 18 + (rand() % 15);
    c.money = 500 + (rand() % 1500);
    strcpy(c.job, "Freelancer");
    c.mood = 50 + (rand() % 50);
    c.addiction = rand() % 40;
    c.curiosity = rand() % 100;
    c.salary = 2000 + (rand() % 4000);
    c.savings = 0;
    c.energy = 50 + (rand() % 50);
    c.hunger = rand() % 50;
    c.sleepiness = rand() % 50;
    c.health = 50 + (rand() % 50);
    c.music_skill = rand() % 100;
    c.fame = rand() % 10;
    for (int i = 0; i < MAX_CITIZENS; i++) c.relationships[i] = 0;
    return c;
}

void handle_social_interactions(struct City *city)
{
    for (int i = 0; i < city->citizen_count; i++)
    {
        struct Citizen *c1 = city->citizens[i];

        // Долги
        if (c1->money < city->food_price)
        {
            for (int j = 0; j < city->citizen_count; j++)
            {
                if (i == j) continue;
                struct Citizen *c2 = city->citizens[j];
                if (c1->relationships[j] > 20 && c2->money > city->food_price * 2)
                {
                    c2->money -= city->food_price;
                    c1->money += city->food_price;
                    c1->relationships[j] += 5;
                    c2->relationships[i] += 5;
                    printf("\n[Social]: %s borrowed money for food from friend %s.\n", c1->name, c2->name);
                    break;
                }
            }
        }

        // Вражда
        for (int j = i + 1; j < city->citizen_count; j++)
        {
            if (c1->relationships[j] < -20)
            {
                printf("\n[Conflict]: %s and %s are enemies! They had a fight today.\n", c1->name, city->citizens[j]->name);
                c1->mood -= 10; if (c1->mood < 0) c1->mood = 0;
                c1->energy -= 10; if (c1->energy < 0) c1->energy = 0;
            }
        }
    }
}

// --- ДВИЖОК СИМУЛЯЦИИ ---

void simulate_day(struct City *city, int day)
{
    printf("\n\n========== DAY %d ==========\n", day);
    int actions[MAX_CITIZENS]; 

    for (int i = 0; i < city->citizen_count; i++)
    {
        struct Citizen *citizen = city->citizens[i];
        int decision = make_decision(citizen);
        actions[i] = decision;
        printf("\n%s decision: %s\n", citizen->name, decision_name(decision));
        execute_decision(citizen, decision);
        buy_food(citizen, city);
        update_health(citizen);
    }

    handle_social_interactions(city);

    for (int i = 0; i < city->citizen_count; i++)
    {
        for (int j = i + 1; j < city->citizen_count; j++)
        {
            if (actions[i] == actions[j] && actions[i] != 0) 
            {
                struct Citizen *c1 = city->citizens[i];
                struct Citizen *c2 = city->citizens[j];
                c1->relationships[j] += 2;
                c2->relationships[i] += 2;
                printf("[Social]: %s and %s spent time together.\n", c1->name, c2->name);

                if (actions[i] == 3 && c1->music_skill > 50 && c2->music_skill > 50) 
                {
                    int bonus = 100; 
                    c1->money += bonus; c2->money += bonus;
                    c1->fame += 5; c2->fame += 5;
                    printf("[Project]: %s and %s made a joint track! Bonus: %d money and +5 fame!\n", c1->name, c2->name, bonus);
                }
            }
        }
    }

    for (int i = 0; i < city->citizen_count; i++)
    {
        struct Citizen *citizen = city->citizens[i];
        citizen->energy += 40;
        if (citizen->energy > 100) citizen->energy = 100;
        citizen->sleepiness = 0;
    }

    if (day % 30 == 1)
    {
        printf("\n========== RENT DAY ==========\n");
        for (int i = 0; i < city->citizen_count; i++) pay_rent(city->citizens[i], city);
    }
}

// --- ГЛАВНАЯ ФУНКЦИЯ ---

int main(void)
{
    srand((unsigned int)time(NULL));

    struct Citizen genry = {"Genry", 20, 1000, "Engineer", 70, 30, 50, 5000, 0, 50, 50, 50, 85, 10, 0, {0}};
    struct Citizen Alex = {"Alex", 25, 1500, "Designer", 80, 20, 60, 4000, 0, 20, 60, 40, 55, 20, 0, {0}};
    struct Citizen Mia = {"Mia", 19, 700, "Artist", 90, 10, 80, 3000, 40000, 35, 80, 10, 100, 80, 5, {0}};

    struct City palaroid;
    strcpy(palaroid.name, "Palaroid");
    palaroid.food_price = 50;
    palaroid.rent_price = 500;
    palaroid.capacity = 3; 
    palaroid.citizen_count = 0;
    palaroid.citizens = malloc(palaroid.capacity * sizeof(struct Citizen*));

    add_citizen(&palaroid, &genry);
    add_citizen(&palaroid, &Alex);
    add_citizen(&palaroid, &Mia);

    char *new_names[10] = {"Ivan", "Elena", "Max", "Sofia", "Dmitry", "Anna", "Pavel", "Ksenia", "Artem", "Vika"};
    for (int i = 0; i < 10; i++)
    {
        struct Citizen *ptr = malloc(sizeof(struct Citizen));
        *ptr = create_random_citizen(new_names[i]); 
        add_citizen(&palaroid, ptr);
    }

    system("cls");
    city_status(palaroid);
    printf("\nPress ENTER to start Day 30...");
    getchar();

    int current_day = 30; 
    while (1) 
    {
        system("cls");
        simulate_day(&palaroid, current_day);
        printf("\n========================================\n");
        printf(" [Day %d finished. Press ENTER for Day %d]\n", current_day, current_day + 1);
        getchar(); 
        current_day++; 
    }
    
    return 0;
}