#include <iostream>

using namespace std;

bool isNumber(int number)
{
    int divisor = 100000;
    int lastDigit = -1;
    bool digitsRepeat = false;
    //cout << "Checking number: " << number << endl;
    while (divisor > 0)
    {
        int digit = number / divisor;
        //cout << "Current digit: " << digit << endl;
        //cout << "Current divisor: " << divisor << endl;
            
        if (digit < lastDigit)
            return false;
        
        if (digit == lastDigit)
            digitsRepeat = true;
            
        number -= divisor * digit;
        divisor /= 10;
        lastDigit = digit;
    }
    
    return digitsRepeat;
}

int part1(int start, int end)
{
    int count = 0;
    for (int i = start; i <= end; i++)
    {
        if (isNumber(i)) count++;
    }
    return count;
}

int isBetterNumber(int number)
{
    int divisor = 100000;
    int lastDigit = -1;
    bool digitsRepeat = false;
    int sameDigitRepeatCount = 0;
    //cout << "Checking number: " << number << endl;
    while (divisor > 0)
    {
        int digit = number / divisor;
        //cout << "Current digit: " << digit << endl;
        //cout << "Current divisor: " << divisor << endl;
            
        if (digit == lastDigit)
            ;// do nothing ;
        else if (digit < lastDigit)
            return false;
        else
        {
            if (sameDigitRepeatCount == 2)
                digitsRepeat = true;
            sameDigitRepeatCount = 0;   
        }
        
        sameDigitRepeatCount++;
        number -= divisor * digit;
        divisor /= 10;
        lastDigit = digit;
    }
    
    if (sameDigitRepeatCount == 2)
        digitsRepeat = true;
                
    return digitsRepeat;
}

int part2(int start, int end)
{
    int count = 0;
    for (int i = start; i <=end; i++)
    {
        if (isBetterNumber(i)) 
        {
            count++;
            //cout << i << endl;
        }
    }
    return count;
}

int main()
{
    isBetterNumber(777888);
    isBetterNumber(123444);
    isBetterNumber(111122);
    
    cout << "Number of passwords: " << part1(265275, 781584) << endl;
    cout << "Number of passwords: " << part2(265275, 781584) << endl;
    return 0;
}
