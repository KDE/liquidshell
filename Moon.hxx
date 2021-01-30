/// Original code from: http://www.acmesoftwareworks.com/moon/
/// Created March 31, 2004 by Don Dugger
/// Modified for liquidshell by Martin Koller, 30.Jan 2021
///
/////////////////////////////////////////////////////////////////////////
///
/// <PRE>
/// THE SOFTWARE IS PROVIDED ~AS IS~, WITHOUT WARRANTY OF ANY KIND,
/// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
/// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
/// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
/// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
/// DEALINGS IN THE SOFTWARE.
/// </PRE>
///
/////////////////////////////////////////////////////////////////////////

#ifndef _moon_h_
#define _moon_h_

#include <QDate>

/////////////////////////////////////////////////////////////////////////
/// <STRONG>
/// Classes
/// </STRONG>
/// <p>
///
///   This class is an all static.
/// </p><p>
///   This group of metthods deals with the phases of the moon, it also computes<BR>
///   Julian dates, due to the fact it was needed for the moon calculations. To make it<BR>
///   simpler to work with the Julian calculation there left in rather than creating a<BR>
///   separate class.
/// </p><p>
///   Some functions are taken from "Numerical Recipes in C - The Art of Scientific Computing"
///   Second Edition, tranlated with changes for C++ and help clarify the code.
/// <BR>
///   ISBN 0-521-43108-5
/// <BR>
///   (Copyright &copy; Cambridge University Press 1988, 1992)
/// </p><p>
///   \section lunarphase  Lunar Phases
/// </p><p>
///    The program definds the phase of the moon as a floating point number between 0 and 4.
///    The following values apply:
///    - 0.0 = New Moon
///    - 1.0 = 1st Quarter
///    - 2.0 = Full Moon
///    - 3.0 = Last Quarter
/// </p>
///@brief Calculate the phase of the moon.
///
/////////////////////////////////////////////////////////////////////////
class Moon {
    /// Radians to degree convertion
    static const double RAD;
    /// We will call this the Gregorian Interval.
    /// The Gregorian Calendar was adopted in October 15, 1582.
    static const long IGREG;
    // The mean lunar cycle
    static const double MEAN_LUNAR_CYCLE;

public:

    Moon() = delete;
    ~Moon() = delete;

    /////////////////////////////////////////////////////////////////////////
    /// phase() calculates the phase of the moon at
    /// noon of the date given.
    ///<p> <i>See above</i> @ref lunarphase
    ///@return  The Lunar Phase <i>see above</i> \ref lunarphase
    static double phase(const QDate &date);

    /////////////////////////////////////////////////////////////////////////
    /// flmoon() calculates the julian date of nth lunar cycle and nph phase
    /// of the moon since Jan 1900
    /// n is the number of lunar cycle since Jan 1900 - ???
    /// This is the original function from the book.
    /// It's been modified to work in C++.
    /// I also rewrote it to be litte clearer. And added
    /// comments where I unterstood what was going on and 
    /// increased the accuracy of a few constants.
    ///@param n     The number of lunar cycles.
    ///@param nph   The lunar phase.
    ///@param jd    The Julian date number.
    ///@param frac  The tractional part of the Julian date number.
    ///@return      The full Julian date of the nth lunar cycle plus nph phase
    ///             Note that the integer and fractional parts are
    ///             returned by references.
    static double flmoon(int n,int nph,long& jd,double& frac);
};

#endif
