#include <QDate>

class Moon
{
  public:
    // returns the frame to be used from the moon_56frames.png
    static int phase(const QDate &date);

  private:
    static double sun_position(double j);
    static double moon_position(double j, double ls);
};
