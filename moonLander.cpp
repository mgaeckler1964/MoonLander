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

/*
	mission metrics:
*/
const double START_HEIGHT = 1000;		// here we start [meter]
const double START_FUEL = 1000;			// this is the amount of fuel [liter]
const double MAX_SPEED = 5;				// max. allowed landing speed [m/s]
										// Note: This is a more modern version 
										// of the linar lander,  the original 
										// eagle allows 1m/s, only

const double BRAKE_FACTOR = 0.3;		// accelleration for one brake rocket level
const double CONSUMPTION_FACTOR = 10;	// fuel-consumption per 1 m/s² and 1 s

// --------------------------------------------------------------------- //
// ----- macros -------------------------------------------------------- //
// --------------------------------------------------------------------- //

inline double accelleration( double height )
{
	const double dist = RADIUS_MOON + height;
	double accell = GRAVITATION_CONST * MASS_MOON / (dist*dist);
	return accell;
}

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
	gak::StopWatch	m_sw, m_missionTime;
	Bitmap	m_bg;
	Icon	m_eagle,
			m_fire,
			m_crashed;

	int		m_landerX, m_landerY,
			m_landerHeight, m_fireHeight,
			m_fireX;

	int		m_strength,
			m_counter;

	ProcessStatus handleCreate() override;
	ProcessStatus handleRepaint( Device &hDC ) override;
	ProcessStatus handleCharacterInput( int c ) override;
	void handleTimer() override;
	struct CurrentState
	{
		double brake;
		double moonAccell;
		double accell;
	};
	CurrentState getState() const
	{
		CurrentState	state;
		state.brake = m_strength * BRAKE_FACTOR;
		state.moonAccell = accelleration(m_height);
		state.accell = m_height > 0 ? state.moonAccell - state.brake : 0;

		return state;
	}

	void restart()
	{
		m_landerY = m_landerHeight;

		m_height = START_HEIGHT;
		m_speed = 0;
		m_fuel=START_FUEL;
		m_missionTime.start();
		setTimer(100);
	}
	void stopMission()
	{
		assert( !m_height );
		m_missionTime.stop();
		removeTimer();
	}
	bool isMissionActive() const
	{
		return m_missionTime.isRunning();
	}

	public:
	MoonMainWindow() : OverlappedWindow( nullptr ), 
		m_landerX(0), m_landerY(0), m_fireX(0), m_strength(0), m_counter(0), m_sw(true)
	{
		removeStyle(WS_THICKFRAME|WS_MAXIMIZEBOX);
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
	m_bg = Application::loadBitmap(IDB_RISING_EARTH);
	m_eagle = Application::loadIcon(IDI_MOON_LANDER);
	m_crashed = Application::loadIcon(IDI_CRASHED_LANDER);
	m_fire = Application::loadIcon(IDI_FIRE);

	resize(m_bg.getWidth(), m_bg.getHeight());
	adjustWindoRect();

	Size iconSize = m_eagle.getSize();
	m_landerX = (m_bg.getWidth() - iconSize.width)/2;
	m_landerHeight = iconSize.height;

	iconSize = m_fire.getSize();
	m_fireX = (m_bg.getWidth() - iconSize.width)/2;
	m_fireHeight = iconSize.height;

	restart();
	return psDO_DEFAULT;
}

ProcessStatus MoonMainWindow::handleRepaint( Device &hDC )
{
	const int INSTRUMENT_WIDTH = 215;
	const int INSTRUMENT_HEIGHT = 108;
	const int NUMBER_WIDTH = 6;
	const int NUMBER_PREC = 2;
	const int LINE_HEIGHT = 15;
	const int PADDING = 8;

	CurrentState	state = getState();
	Size			size = getClientSize();
	MemoryDevice	mem( hDC, size );

	mem.drawBitmap( 0, 0, m_bg );

	mem.setMonospacedFont();
	mem.getBrush().create( colors::WHITE );
	mem.rectangle( size.width - INSTRUMENT_WIDTH, 0, size.width, INSTRUMENT_HEIGHT );
	gak::StringBuffer<128>	b;

	const int x = size.width - INSTRUMENT_WIDTH + PADDING;
	int y = PADDING;

	mem.textOut( x, y, 
		STRING("Height:       ")
			.add(gak::formatFloat( m_height, NUMBER_WIDTH, NUMBER_PREC ))
			.add(" m")
	);

	y += LINE_HEIGHT;
	mem.textOut( x, y, 
		STRING("Speed:        ")
			.add(gak::formatFloat( m_speed, NUMBER_WIDTH, NUMBER_PREC ))
			.add(" m/s")
	);

	y += LINE_HEIGHT;
	mem.textOut( x, y, 
		STRING("Moon Accell:  ")
			.add(gak::formatFloat( state.moonAccell, NUMBER_WIDTH, NUMBER_PREC ))
			.add(" m/s²")
	);

	y += LINE_HEIGHT;
	mem.textOut( x, y, 
		STRING("Cur Accell:   ")
			.add(gak::formatFloat( state.accell, NUMBER_WIDTH, NUMBER_PREC ))
			.add(" m/s²") 
	);

	y += LINE_HEIGHT;
	mem.textOut( x, y, 
		STRING("Mission Time: ")
			.add(gak::formatNumber( m_missionTime.get<gak::Seconds<>>().get(), NUMBER_WIDTH, ' ' ))
			.add(" s")
	);

	y += LINE_HEIGHT;
	mem.textOut( x, y, 
		STRING("Fuel:         ")
			.add(gak::formatFloat( m_fuel, NUMBER_WIDTH, NUMBER_PREC ))
			.add(" l")
	);

	if( m_landerY >= m_bg.getHeight()-m_landerHeight )
	{
		bool crashed = m_speed >= MAX_SPEED;
		m_landerY = m_bg.getHeight()-m_landerHeight;
		mem.selectFont( Font(this).setVariableFont().setFontSize(20).setBold() );
		mem.setTextColor( winlib::colors::WHITE );
		mem.setTextAlignment( Device::haCenter, Device::vaBaseline );
		mem.setBackgroundColor( winlib::colors::BLACK, TRANSPARENT );
		mem.textOut(size.width/2, size.height/2, crashed ? "Eagle Crashed!" : "Eagle Landed!" );
		mem.drawIcon( m_landerX, m_landerY, crashed ? m_crashed : m_eagle );
	}
	else 
	{
		assert( m_landerX && m_landerY );	// ensure that WM_REDRAW never comes before WM_CREATE
		mem.drawIcon( m_landerX, m_landerY, m_eagle );
		if( m_strength )
		{
			y = m_landerY + m_landerHeight;
			int count = m_counter%(m_strength+1);
			for( int i=0; i<count; ++i )
			{
				mem.drawIcon( m_fireX, y, m_fire );
				y += m_fireHeight;
			}
		}
	}

	mem.drawToWindow();

	return psPROCESSED;
}

void MoonMainWindow::handleTimer()
{
	if( !isMissionActive() )
	{
		assert( !m_height );
		return;		
	}

	CurrentState state = getState();

	double ellapsedTime = double(m_sw.getMillis()) / 1000.0;
	m_sw.start();
	m_height -= (m_speed * ellapsedTime) + (0.5 * state.accell * ellapsedTime * ellapsedTime);
	m_speed += state.accell * ellapsedTime;

	double consumption = state.brake * ellapsedTime * CONSUMPTION_FACTOR;
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
			if( m_landerY >= m_bg.getHeight()-m_landerHeight )
			{
				m_landerY = m_bg.getHeight()-m_landerHeight;
				stopMission();
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
	else if( c == 'r'  )
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

