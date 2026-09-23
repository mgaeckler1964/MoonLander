/*
		Project:		Moon Lander
		Module:			moonLander.cpp
		Description:	A Moon Lander Simulation 
		Author:			Martin Gäckler
		Address:		Hofmannsthalweg 14, A-4030 Linz
		Web:			https://www.gaeckler.at/

		Copyright:		(c) 2026 Martin Gäckler

		This program is free software: you can redistribute it and/or modify  
		it under the terms of the GNU General Public License as published by  
		the Free Software Foundation, version 3.

		You should have received a copy of the GNU General Public License 
		along with this program. If not, see <http://www.gnu.org/licenses/>.

		THIS SOFTWARE IS PROVIDED BY Martin Gäckler, Linz, Austria ``AS IS''
		AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
		TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
		PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR
		CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
		SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
		LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
		USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
		ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
		OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
		SUCH DAMAGE.
*/

// --------------------------------------------------------------------- //
// ----- switches ------------------------------------------------------ //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- includes ------------------------------------------------------ //
// --------------------------------------------------------------------- //

#include <gak/fmtNumber.h>
#include <gak/stopWatch.h>

#include <WINLIB/WINAPP.H>
#include <winlib/POPUP.H>
#include <WINLIB/DEVICE.H>
#include <WINLIB/colors.h>

#include "moonLander_rc.h"

// --------------------------------------------------------------------- //
// ----- imported datas ------------------------------------------------ //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- module switches ----------------------------------------------- //
// --------------------------------------------------------------------- //

#ifdef __BORLANDC__
#	pragma option -RT-
#	pragma option -b
#	pragma option -a4
#	pragma option -pc
#endif

using namespace winlib;

// --------------------------------------------------------------------- //
// ----- constants ----------------------------------------------------- //
// --------------------------------------------------------------------- //

const double GRAVITATION_CONST = 6.674e-11;
const double MASS_MOON = 7.342e22;
const double RADIUS_MOON = 1737400;

// --------------------------------------------------------------------- //
// ----- macros -------------------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- type definitions ---------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class definitions --------------------------------------------- //
// --------------------------------------------------------------------- //

class MoonMainWindow : public OverlappedWindow
{
	double	m_height,
			m_speed,
			m_fuel;
	gak::StopWatch	m_sw, m_landingTime;
	Bitmap	m_bg;
	Icon	m_eagle,
			m_fire;

	int		m_landerX, m_landerY,
			m_landerHeight, m_fireHeight;

	int		m_strength,
			m_counter;

	ProcessStatus handleCreate() override;
	ProcessStatus handleRepaint( Device &hDC ) override;
	ProcessStatus handleCharacterInput( int c ) override;

	void handleTimer() override;
	void restart()
	{
		Size	iconSize = m_eagle.getSize();
		m_landerX = (m_bg.getWidth() - iconSize.height)/2;
		m_landerY = m_landerHeight = iconSize.height;

		m_height = 1000;
		m_speed = 0;
		m_fuel=1000;
		m_landingTime.stop();
		m_landingTime.start();
	}

	public:
	MoonMainWindow() : OverlappedWindow( nullptr ), 
		m_landerX(0), m_landerY(0), m_strength(0), m_counter(0), m_sw(true)
	{
		removeStyle(WS_THICKFRAME);
		setText("Moon Lander");
		restart();
	}

};

class WindowsApplication : public Application
{
	bool startApplication( HINSTANCE /*hInstance*/, const char * /*cmdLine*/ ) override
	{
		doEnableLogEx(gakLogging::llInfo);
		doDisableLog();
		setApplication("MoonLander");
		setCompany("gak");
		return 0;
	}
	CallbackWindow *createMainWindow( const char * /*cmdLine*/, int /*nCmdShow*/ ) override
	{
		std::unique_ptr<MoonMainWindow>	mainWindow( new MoonMainWindow );
		if( mainWindow->create( nullptr ) == scERROR )
		{
			throw gak::LibraryException( "Could not create window!" );
		}

		mainWindow->focus();
		return mainWindow.release();
	}
	void deleteMainWindow( BasicWindow  *mainWindow ) override
	{
		delete mainWindow;
	}

	public:
	WindowsApplication() : Application( IDI_MOON_LANDER ) {}
};

// --------------------------------------------------------------------- //
// ----- exported datas ------------------------------------------------ //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- module static data -------------------------------------------- //
// --------------------------------------------------------------------- //

static WindowsApplication	app;

// --------------------------------------------------------------------- //
// ----- class static data --------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- prototypes ---------------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- module functions ---------------------------------------------- //
// --------------------------------------------------------------------- //

static double accelleration( double height )
{
	const double dist = RADIUS_MOON + height;
	double accell = GRAVITATION_CONST * MASS_MOON / (dist*dist);
	return accell;
}

// --------------------------------------------------------------------- //
// ----- class inlines ------------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class constructors/destructors -------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class static functions ---------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class privates ------------------------------------------------ //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class protected ----------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class virtuals ------------------------------------------------ //
// --------------------------------------------------------------------- //
   
ProcessStatus MoonMainWindow::handleCreate()
{
	m_bg = Application::loadBitmap(ID_BMP_MOON_LANDER);
	m_eagle = Application::loadIcon(IDI_MOON_LANDER);
	m_fire = Application::loadIcon(IDI_FIRE);

	resize(m_bg.getWidth(), m_bg.getHeight());
	adjustWindoRect();

	Size iconSize = m_fire.getSize();
	m_fireHeight = iconSize.height;

	setTimer(100);

	restart();
	return psDO_DEFAULT;
}

ProcessStatus MoonMainWindow::handleRepaint( Device &hDC )
{
	const int DISPLAY_WIDTH = 215;
	const int DISPLAY_HEIGHT = 108;
	const int NUMBER_WIDTH = 6;

	Size			size = getClientSize();
	RectBorder		rect = getClientRectangle();
	MemoryDevice	mem( hDC, size );

	mem.drawBitmap( 0, 0, m_bg );

	mem.setMonospacedFont();
	mem.getBrush().create( colors::WHITE );
	mem.rectangle( size.width - DISPLAY_WIDTH, 0, size.width, DISPLAY_HEIGHT );
	gak::StringBuffer<128>	b;

	int x = size.width - DISPLAY_WIDTH + 8;
	int y = 8;

	double brake = m_strength * 0.3;
	double moonAccell = accelleration(m_height);
	double accell = m_height > 0 ? moonAccell - brake : 0;

	mem.textOut( x, y, 
		STRING("Height:       ")
			.add(gak::formatFloat( m_height, NUMBER_WIDTH, 1 ))
			.add(" m")
	);

	y += 15;
	mem.textOut( x, y, 
		STRING("Speed:        ")
			.add(gak::formatFloat( m_speed, NUMBER_WIDTH, 1 ))
			.add(" m/s")
	);

	y += 15;
	mem.textOut( x, y, 
		STRING("Moon Accel:   ")
			.add(gak::formatFloat( moonAccell, NUMBER_WIDTH, 1 ))
			.add(" m/s²")
	);

	y += 15;
	mem.textOut( x, y, 
		STRING("Cur Accel:    ")
			.add(gak::formatFloat( accell, NUMBER_WIDTH, 1 ))
			.add(" m/s²") 
	);

	y += 15;
	mem.textOut( x, y, 
		STRING("Mission Time: ")
			.add(gak::formatNumber( m_landingTime.get<gak::Seconds<>>().get(), NUMBER_WIDTH, ' ' ))
			.add(" s")
	);

	y += 15;
	mem.textOut( x, y, 
		STRING("Fuel:         ")
			.add(gak::formatFloat( m_fuel, NUMBER_WIDTH, 1 ))
			.add(" l")
	);

	if( m_landerY >= m_bg.getHeight()-m_landerHeight )
	{
		mem.selectFont( Font(this).setVariableFont().setFontSize(20).setBold() );
		mem.setTextColor( winlib::colors::WHITE );
		mem.setTextAlignment( Device::haCenter, Device::vaBaseline );
		mem.setBackgroundColor( winlib::colors::BLACK, TRANSPARENT );
		mem.textOut(rect.right/2, rect.bottom/2, m_speed < 5 ? "Eagle landed!" : "Eagle Crashed!" );
	}

	if( m_landerX && m_landerY )
		mem.drawIcon( m_landerX, m_landerY, m_eagle );
	if( m_strength )
	{
		x = m_landerX;
		y = m_landerY + m_landerHeight;
		int count = m_counter%(m_strength+1);
		for( int i=0; i<count; ++i )
		{
			mem.drawIcon( x, y, m_fire );
			y += m_fireHeight;
		}
	}

	mem.drawToWindow();

	return psPROCESSED;
}

void MoonMainWindow::handleTimer()
{
	double brake = m_strength * 0.3;
	double moonAccell = accelleration(m_height);
	double accell = m_height > 0 ? moonAccell - brake : 0;

	double ellapsedTime = double(m_sw.getMillis()) / 1000.0;
	m_sw.stop();
	m_sw.start();
	m_speed += accell * ellapsedTime;
	m_height -= m_speed * ellapsedTime;

	double consumption = brake * ellapsedTime * 10;
	m_fuel -= consumption;
	if( m_fuel <= 0 )
	{
		m_fuel = 0;
		m_strength = 0;
	}
	if( m_height <= 0 )
	{
		m_height = 0;
		m_strength = 0;
		if( m_landerY < m_bg.getHeight()-m_landerHeight )
		{
			m_landerY += int(m_speed);
			if( m_landerY > m_bg.getHeight()-m_landerHeight )
			{
				m_landerY = m_bg.getHeight()-m_landerHeight;
			}
		}
	}

	++m_counter;
	invalidateWindow( false );
}

ProcessStatus MoonMainWindow::handleCharacterInput( int c )
{
	if( c >= '0' && c <= '9' && m_fuel > 0 )
		m_strength = c - '0';

	if( c == 'r'  )
		restart();
	return psPROCESSED;
}

// --------------------------------------------------------------------- //
// ----- class publics ------------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- entry points -------------------------------------------------- //
// --------------------------------------------------------------------- //

#ifdef __BORLANDC__
#	pragma option -RT.
#	pragma option -b.
#	pragma option -a.
#	pragma option -p.
#endif

