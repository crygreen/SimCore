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

void change_mood(struct Citizen *citizen, int new_mood)
{
    citizen->mood = new_mood;
}

void citizen_status(struct Citizen citizen)
{
    printf("\n%s is %d years old.\n", citizen.name, citizen.age);
    printf("Job: %s\n", citizen.job);
    printf("Money: %d\n", citizen.money);
    printf("Mood: %d\n", citizen.mood);
    printf("Energy: %d\n", citizen.energy);
    printf("Health: %d\n", citizen.health);
}

void add_money(struct Citizen *citizen, int amount)
{
    citizen->money += amount;
}

void work(struct Citizen *citizen, int hours)
{
    if (citizen->energy < 20)
    {
        printf("\n%s is too tired to work.\n", citizen->name);
        return;
    }
    if (citizen->health < 30)
    {
        printf("\n%s is too unhealthy to work.\n", citizen->name);
        return;
    }

    int income = hours * citizen->salary / 160;
    if (citizen->health < 60)
    {
        income = income * 70 / 100;
    }

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
    if (citizen->money >= amount)
    {
        citizen->money -= amount;
        return 1;
    }
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
    else
    {
        printf("\n%s cannot afford food.\n", citizen->name);
    }
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
    printf("City: %s\n", city.name);
    printf("Food price: %d\n", city.food_price);
    printf("Rent price: %d\n", city.rent_price);
    printf("Total citizens: %d\n", city.citizen_count);
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

int make_decision(struct Citizen *citizen)
{
    int weights[3] = {30, 50, 50};
    weights[1] += citizen->curiosity;

    if (citizen->mood < 50) weights[0] += 50 - citizen->mood;
    weights[2] -= citizen->addiction;
    weights[0] += citizen->addiction / 2;

    if (citizen->health < 60)
    {
        weights[0] += 60 - citizen->health;
        weights[2] -= 60 - citizen->health;
    }

    if (weights[0] < 0) weights[0] = 0;
    if (weights[1] < 0) weights[1] = 0;
    if (weights[2] < 0) weights[2] = 0;

    int total_weight = weights[0] + weights[1] + weights[2];
    if (total_weight == 0) return 0;

    int random_value = rand() % total_weight;
    int accumulated = 0;

    for (int i = 0; i < 3; i++)
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
    return "Unknown decision";
}

void execute_decision(struct Citizen *citizen, int decision)
{
    if (decision == 0)
    {
        printf("\n%s stayed at home.\n", citizen->name);
        rest(citizen);
    }
    else if (decision == 1)
    {
        printf("\n%s went outside.\n", citizen->name);
        citizen->mood += 5;
        if (citizen->mood > 100) citizen->mood = 100;
    }
    else if (decision == 2)
    {
        printf("\n%s decided to work.\n", citizen->name);
        work(citizen, 8);
    }
}

// ВОТ ОНА, НАША НОВАЯ ФУНКЦИЯ ДОБАВЛЕНИЯ ЖИТЕЛЯ
void add_citizen(struct City *city, struct Citizen *new_citizen)
{
    if (city->citizen_count >= city->capacity)
    {
        city->capacity = city->capacity * 2;
        city->citizens = realloc(city->citizens, city->capacity * sizeof(struct Citizen*));
        
        if (city->citizens == NULL) {
            printf("Ошибка: не удалось выделить память!\n");
            exit(1);
        }
        printf("[System]: Город расширен до %d мест.\n", city->capacity);
    }

    city->citizens[city->citizen_count] = new_citizen;
    city->citizen_count++;
}

void simulate_day(struct City *city, int day)
{
    printf("\n\n========== DAY %d ==========\n", day);

    for (int i = 0; i < city->citizen_count; i++)
    {
        struct Citizen *citizen = city->citizens[i];
        
        int decision = make_decision(citizen);
        printf("\n%s decision: %s\n", citizen->name, decision_name(decision));
        execute_decision(citizen, decision);

        buy_food(citizen, city);
        update_health(citizen);
    }

    if (day % 30 == 1)
    {
        printf("\n========== RENT DAY ==========\n");
        for (int i = 0; i < city->citizen_count; i++)
        {
            pay_rent(city->citizens[i], city);
        }
    }
}

int main(void)
{
    srand((unsigned int)time(NULL));

    struct Citizen genry = {
        "Genry", 20, 1000, "Engineer", 70, 30, 50,
        5000, 0, 50, 50, 50, 85, {0}
    };

    struct Citizen Alex = {
        "Alex", 25, 1500, "Designer", 80, 20, 60,
        4000, 0, 20, 60, 40, 55, {0}
    };

    struct Citizen Mia = {
        "Mia", 19, 700, "Artist", 90, 10, 80,
        3000, 40000, 35, 80, 10, 100, {0}
    };

    // Настройка города
    struct City palaroid;
    strcpy(palaroid.name, "Palaroid");
    palaroid.food_price = 50;
    palaroid.rent_price = 500;
    palaroid.capacity = 3; 
    palaroid.citizen_count = 0;
    
    // Выделяем память
    palaroid.citizens = malloc(palaroid.capacity * sizeof(struct Citizen*));

    // Добавляем жителей
    add_citizen(&palaroid, &genry);
    add_citizen(&palaroid, &Alex);
    add_citizen(&palaroid, &Mia);

    city_status(palaroid);

    printf("\nStarting citizens:\n");
    for (int i = 0; i < palaroid.citizen_count; i++) {
        citizen_status(*palaroid.citizens[i]);
    }

    for (int day = 1; day <= DAYS_TO_SIMULATE; day++) {
        simulate_day(&palaroid, day);
    }

    printf("\n\n========== FINAL CITY REPORT ==========\n");
    for (int i = 0; i < palaroid.citizen_count; i++) {
        citizen_status(*palaroid.citizens[i]);
    }

    // Очистка памяти
    free(palaroid.citizens);

    return 0;
}