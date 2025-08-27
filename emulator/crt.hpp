#pragma once

#include <cassert>
#include <cstdint>

#include "memory.hpp"
#include "iw.hpp"

#include "addrs.hpp"

class crt_t
{
	const memory_t &memory_;

	uint8_t section_; 	/* 0 to 3 */
	uint8_t level_; 	/* 0 to 3 */
	bool interleaved_;  /* Interleaved display mode */
	bool underline_;	/* Underline caracters if needed */
	bool disable_;      /* Disable display */
	bool mode4lines_;   /* Displays only 4 lines */

	addrs_t screen_;	/* Start of screen memory */
	addrs_t font_; 		/* Start of font memory */

	public:
	crt_t( const memory_t &memory)
		: memory_(memory)
		, section_(0)
		, level_(0)
		, interleaved_(false)
		, underline_(false)
		, disable_(false)
		, mode4lines_(false)
		, screen_{ section_, level_, 0 }
		, font_{ 4, 0, 0 }
		{
		}

	static const int char_width_ = 5;
	static const int char_height_ = 8;
	static const int matrix_width_ = 8;
	static const int matrix_height_ = 16;
	static const int screen_columns_ = 32;
	static const int screen_lines_ = 8;

    void execute( uint8_t value )
	{
		section_ = (value >> 6) & 0b11;
		level_ = (((value >> 5) &0xb1) << 1 ) | ((value >> 1) & 0b1);
		interleaved_ = !!(value & 0b00010000);
		underline_ = !!(value & 0b00001000);
		disable_ = !!(value & 0b00000100);
		mode4lines_ = !!(value & 0b00000001);

		screen_ = addrs_t(section_, level_, 0);
	}

	typedef char screen_buffer_t[matrix_height_*screen_lines_][matrix_width_*screen_columns_];

	void render( screen_buffer_t screen, int x, int y, uint8_t font_byte ) const
	{
		for (auto bit=0;bit!=8;bit++)
		{
			// std::cout << y << " " << std::flush;
			assert( x >= 0 && x < (matrix_width_*screen_columns_) );
			assert( y+bit >= 0 && y+bit < (matrix_height_*screen_lines_) );
			screen[y+bit][x] = (font_byte & (1<<(7-bit))) ? '*' : ' ';
		}
	}

	void render( screen_buffer_t screen ) const
	{
		int x = 0;

		for (auto column=0;column!=screen_columns_;column++)
		{
			// Each char is 5 pixels wide
			for (auto charcol=0;charcol!=char_width_;charcol++)
			{
				int y = 0;
				/// There are 8 lines of chars on screen
				for (auto line=0;line!=screen_lines_;line++)
				{
					auto char_addr = screen_ + column + line*screen_columns_;
					auto ch = memory_[char_addr];
					auto font_addr = font_ + (ch*char_width_) + charcol;
					auto font_byte = memory_[font_addr];

					render( screen, x, y, font_byte );
					y += 8;
					render( screen, x, y, 0xff );
					y += 8;
				}
				x++;
			}

			for (auto charcol=char_width_;charcol!=matrix_width_;charcol++)
			{
				int y = 0;
				/// There are 8 lines of chars on screen
				for (auto line=0;line!=screen_lines_;line++)
				{
					render( screen, x, y, 0xff );
					y += 8;
					render( screen, x, y, 0xff );
					y += 8;
				}
				x++;
			}
		}
	}

};
