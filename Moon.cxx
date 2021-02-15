// original code from http://www.voidware.com/phase.c
// adjusted for liquidshell

#include <Moon.hxx>
#include <cmath>

#define PI  3.1415926535897932384626433832795
#define RAD  (PI/180.0)
#define SMALL_FLOAT  (1e-12)

//--------------------------------------------------------------------------------

double Moon::phase(const QDate &date)
{
  double j = date.toJulianDay() - 2444238.5;
  double ls = sun_position(j);
  double lm = moon_position(j, ls);
  double t = lm - ls;

  if (t < 0) t += 360;

  return (1.0 - std::cos((lm - ls) * RAD)) / 2;
}

//--------------------------------------------------------------------------------

double Moon::sun_position(double j)
{
  double n, x, e, l, dl, v;
  int i;
  n = 360 / 365.2422 * j;
  i = n / 360;
  n = n - i * 360.0;
  x = n - 3.762863;

  if (x < 0) x += 360;

  x *= RAD;
  e = x;

  do
  {
    dl = e - .016718 * std::sin(e) - x;
    e = e - dl / (1 - .016718 * std::cos(e));
  }
  while (std::fabs(dl) >= SMALL_FLOAT);

  v = 360 / PI * std::atan(1.01686011182 * std::tan(e / 2));
  l = v + 282.596403;
  i = l / 360;
  l = l - i * 360.0;
  return l;
}

//--------------------------------------------------------------------------------

double Moon::moon_position(double j, double ls)
{
  double ms, l, mm, n, ev, sms, ae, ec;
  int i;
  /* ls = sun_position(j) */
  ms = 0.985647332099 * j - 3.762863;

  if (ms < 0) ms += 360.0;

  l = 13.176396 * j + 64.975464;
  i = l / 360;
  l = l - i * 360.0;

  if (l < 0) l += 360.0;

  mm = l - 0.1114041 * j - 349.383063;
  i = mm / 360;
  mm -= i * 360.0;
  n = 151.950429 - 0.0529539 * j;
  i = n / 360;
  n -= i * 360.0;
  ev = 1.2739 * std::sin((2 * (l - ls) - mm) * RAD);
  sms =  std::sin(ms * RAD);
  ae = 0.1858 * sms;
  mm += ev - ae - 0.37 * sms;
  ec = 6.2886 * std::sin(mm * RAD);
  l += ev + ec - ae + 0.214 * std::sin(2 * mm * RAD);
  l = 0.6583 * std::sin(2 * (l - ls) * RAD) + l;
  return l;
}

//--------------------------------------------------------------------------------
