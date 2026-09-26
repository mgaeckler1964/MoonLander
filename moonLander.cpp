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
#include <gak/physic.h>
#include <gak/optional.h>

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

// --------------------------------------------------------------------- //
// ----- type definitions ---------------------------------------------- //
// --------------------------------------------------------------------- //

// --------------------------------------------------------------------- //
// ----- class definitions --------------------------------------------- //
// --------------------------------------------------------------------- //

class MoonMainWindow : public OverlappedWindow
{
	gak::PODarray<double>		m_heights, m_speeds;
	gak::math::MinMax<double>	m_heightRange, m_speedRange;

	double	m_height,
			m_speed,
			m_fuel;
	gak::StopWatch	m_sw, m_missionTime;
	Bitmap	m_bg;
	Icon	m_eagle,
			m_fire,
			m_crashed,
			m_austria,
			m_eu;

	int		m_landerX, m_landerY,
			m_landerHeight, m_fireHeight, m_austriaHeight, m_euHeight,
			m_fireX;

	int		m_strength,
			m_counter;

	bool	m_showEstimations, m_showGraph;

	void paintMoon( MemoryDevice &hDC );
	void paintGraph( MemoryDevice &hDC );
	static void paintGraph( MemoryDevice &hDC, const gak::PODarray<double> &data, const gak::Duo<double,double> &range);

	ProcessStatus handleCreate() override;
	ProcessStatus handleRepaint( Device &hDC ) override;
	ProcessStatus handleCharacterInput( int c ) override;
	void handleTimer() override;
	struct CurrentState
	{
		double brake;
		double moonAccel;
		double accel;
	};
	CurrentState getState() const
	{
		CurrentState	state;
		state.brake = m_strength * BRAKE_FACTOR;
		state.moonAccel = gak::physic::moonAcceleration(m_height);
		state.accel = m_height > 0 ? state.moonAccel - state.brake : 0;

		return state;
	}

	void restart()
	{
		m_landerY = m_landerHeight;

		m_height = START_HEIGHT;
		m_speed = 0;
		m_fuel=START_FUEL;
		m_heights.empty();
		m_speeds.empty();
		m_heightRange.reset();
		m_speedRange.reset();
		m_missionTime.start();
		m_sw.start();
		setTimer(100);
	}
	void stopMission()
	{
		assert( !m_height );
		m_missionTime.stop();
		m_sw.stop();
		removeTimer();
	}
	bool isMissionActive() const
	{
		return m_missionTime.isRunning();
	}

	public:
	MoonMainWindow() : OverlappedWindow( nullptr ), 
		m_landerX(0), m_landerY(0), m_fireX(0), m_strength(0), m_counter(0), m_showEstimations(false), m_showGraph(false)
	{
		removeStyle(WS_THICKFRAME|WS_MAXIMIZEBOX);
		setText("Moon Lander");
		m_heights.setCapacity(1024, false);
		m_speeds.setCapacity(1024, false);
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

void MoonMainWindow::paintMoon( MemoryDevice &mem )
{
	const int PADDING = 8;
	const int NUMBER_WIDTH = 6;
	const int NUMBER_PREC = 2;
	const int LINE_HEIGHT = 15;
	const int INSTRUMENT_WIDTH = 215;
	const int INSTRUMENT_HEIGHT = 2*PADDING+8*LINE_HEIGHT;

	CurrentState	state = getState();

	mem.drawBitmap( 0, 0, m_bg );

	mem.setMonospacedFont();
	mem.getBrush().create( colors::WHITE );
	mem.rectangle( 0, 0, INSTRUMENT_WIDTH, m_showEstimations ? INSTRUMENT_HEIGHT : (INSTRUMENT_HEIGHT-2*LINE_HEIGHT) );
	gak::StringBuffer<128>	b;

	const int x = PADDING;
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
		STRING("Moon Accel.:  ")
			.add(gak::formatFloat( state.moonAccel, NUMBER_WIDTH, NUMBER_PREC ))
			.add(" m/s²")
	);

	y += LINE_HEIGHT;
	mem.textOut( x, y, 
		STRING("Cur. Accel.:  ")
			.add(gak::formatFloat( state.accel, NUMBER_WIDTH, NUMBER_PREC ))
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

	if( m_showEstimations )
	{
		y += LINE_HEIGHT;
		double landingTime = gak::physic::acceleratedTime(m_speed,state.accel,m_height);
		if( landingTime>= 0 )
		{
			mem.textOut( x, y, 
				STRING("Est. Time:    ")
				.add(gak::formatFloat( landingTime, NUMBER_WIDTH, NUMBER_PREC ))
					.add(" s")
			);
		}
		else
		{
			mem.textOut( x, y, STRING("Est. Time:    ------") );
		}

		y += LINE_HEIGHT;
		if( landingTime>= 0 )
		{
			double landingSpeed = gak::physic::speed(m_speed, state.accel, landingTime );
			mem.textOut( x, y, 
				STRING("Est. Speed:   ")
				.add(gak::formatFloat( landingSpeed, NUMBER_WIDTH, NUMBER_PREC ))
					.add(" m/s")
			);
		}
		else
		{
			mem.textOut( x, y, STRING("Est. Speed:   ------") );
		}
	}

	if( m_landerY >= m_bg.getHeight()-m_landerHeight )
	{
		bool crashed = m_speed >= MAX_SPEED;
		m_landerY = m_bg.getHeight()-m_landerHeight;
		mem.selectFont( Font(this).setVariableFont().setFontSize(20).setBold() );
		mem.setTextColor( winlib::colors::WHITE );
		mem.setTextAlignment( Device::haCenter, Device::vaBaseline );
		mem.setBackgroundColor( winlib::colors::BLACK, TRANSPARENT );
		const Size &size = mem.getSize();
		mem.textOut(size.width/2, size.height/2, crashed ? "Eagle Crashed!" : "Eagle Landed!" );
		mem.drawIcon( m_landerX, m_landerY, crashed ? m_crashed : m_eagle );
		if( !crashed )
		{
			int x = m_landerX + 128;
			int y = m_bg.getHeight()-m_austriaHeight;
			mem.drawIcon( x, y, m_austria );

			x = m_landerX - 128;
			y = m_bg.getHeight()-m_euHeight;
			mem.drawIcon( x, y, m_eu );
		}
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
}

void MoonMainWindow::paintGraph( MemoryDevice &mem, const gak::PODarray<double> &data, const gak::Duo<double,double> &range)
{
	const Size &size = mem.getSize();

	gak::Duo<double, double>	screenYrange(size.height, 0),
								screenXIn(0,double(data.size()-1)),
								screenXOut(0,size.width);
	gak::Optional<gak::math::Scale<double>>	xScale;
	
	gak::math::Scale<double>	yScale( range, screenYrange );

	if( int(data.size()) > size.width )
	{
		xScale = gak::math::Scale<double>( screenXIn, screenXOut );
	}

	bool first = true;
	for( int i=0; i<int(data.size()); ++i )
	{
		int screenX = i;
		if( int(data.size()) > size.width )
		{
			screenX = gak::math::round<int>(xScale.get()( double(screenX)));
		}
		int screenY = gak::math::round<int>(yScale( data[i] ));
		if( first )
		{
			mem.moveTo( screenX, screenY );
			first = false;
		}
		else
		{
			mem.lineTo( screenX, screenY );
		}
	}
}

void MoonMainWindow::paintGraph( MemoryDevice &mem )
{
	const Size &size = mem.getSize();

	mem.getBrush().create( colors::WHITE );
	mem.rectangle( 0, 0, size.width, size.height );

	mem.getPen().setColor( colors::BLUE );
	paintGraph( mem, m_heights, m_heightRange.getDuo() );
	mem.getPen().setColor( colors::RED );
	paintGraph( mem, m_speeds, m_speedRange.getDuo() );
}

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
	m_austria = Application::loadIcon(IDI_AUSTRIA);
	m_eu = Application::loadIcon(IDI_EU);

	resize(m_bg.getWidth(), m_bg.getHeight());
	adjustWindoRect();

	Size iconSize = m_eagle.getSize();
	m_landerX = (m_bg.getWidth() - iconSize.width)/2;
	m_landerHeight = iconSize.height;

	iconSize = m_fire.getSize();
	m_fireX = (m_bg.getWidth() - iconSize.width)/2;
	m_fireHeight = iconSize.height;

	iconSize = m_austria.getSize();
	m_austriaHeight = iconSize.height;

	iconSize = m_eu.getSize();
	m_euHeight = iconSize.height;

	restart();
	return psDO_DEFAULT;
}

ProcessStatus MoonMainWindow::handleRepaint( Device &hDC )
{
	Size			size = getClientSize();
	MemoryDevice	mem( hDC, size );
	if( m_showGraph )
		paintGraph(mem);
	else
		paintMoon(mem);

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

	if( m_height > 0 )
	{
		const CurrentState state = getState();

		const double elapsedTime = double(m_sw.getMillis()) / 1000.0;
		m_sw.start();
		m_height -= gak::physic::distance( m_speed, state.accel, elapsedTime );
		m_speed = gak::physic::speed( m_speed, state.accel, elapsedTime );
		if( m_height <= 0 )
		{
			m_height = 0;
			m_strength = 0;
		}
		m_heights.addElement( m_height );
		m_heightRange.test( m_height );
		m_speeds.addElement( m_speed );
		m_speedRange.test( m_speed );

		const double consumption = state.brake * elapsedTime * CONSUMPTION_FACTOR;
		m_fuel -= consumption;
		if( m_fuel <= 0 )
		{
			m_fuel = 0;
			m_strength = 0;
		}
	}

	if( m_height <= 0 )
	{
		m_height = 0;
		m_strength = 0;
		if( m_landerY < m_bg.getHeight()-m_landerHeight )
		{
			m_landerY += gak::math::max(gak::math::round<int>(m_speed),1);
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
	else if( c == 'e'  )
		m_showEstimations = !m_showEstimations;
	else if( c == ' '  )
	{
		m_showGraph = !m_showGraph;
		invalidateWindow();
	}
	else if( c == 'q'  )
		close();
;
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

