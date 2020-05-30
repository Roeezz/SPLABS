int digit_cnt(char *str);

int main(int argc, char **argv)
{
    return 0;
}

int digit_cnt(char *str)
{
    int acc = 0;
    for (int i = 0; str[i] != 0; i++)
    {
        if (str[i] >= 48 && str[i] <= 57)
        {
            acc++;
        }
    }
    return acc;
}