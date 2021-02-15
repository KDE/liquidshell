#include <QDate>

class Moon
{
  public:
    // returns the moon phase as a real number (0-1)
    static double phase(const QDate &date);

  private:
    static double sun_position(double j);
    static double moon_position(double j, double ls);
};
