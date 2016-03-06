/*
 For general Scribus (>=1.3.2) copyright and licensing information please refer
 to the COPYING file provided with the program. Following this notice may exist
 a copyright and/or license notice that predates the release of Scribus 1.3.2
 for which a new license (GPL+exception) is in place.
 */
/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#undef NDEBUG

#include <cassert>
#include "../styles/charstyle.h"
#include "pageitem.h"
#include "prefsstructs.h"
#include "../styles/paragraphstyle.h"
#include "specialchars.h"
#include "storytext.h"
#include "textlayout.h"
#include "boxes.h"


TextLayoutPainter::~TextLayoutPainter()
{ }

void TextLayoutPainter::setFont(const ScFace font)
{
	m_state.font = font;
}

ScFace TextLayoutPainter::font()
{
	return m_state.font;
}

void TextLayoutPainter::setFontSize(double size)
{
	m_state.fontSize = size;
}

double TextLayoutPainter::fontSize()
{
	return m_state.fontSize;
}

void TextLayoutPainter::setStrokeColor(TextLayoutColor color)
{
	m_state.strokeColor = color;
}

TextLayoutColor TextLayoutPainter::strokeColor()
{
	return m_state.strokeColor;
}

void TextLayoutPainter::setFillColor(TextLayoutColor color)
{
	m_state.fillColor = color;
}

TextLayoutColor TextLayoutPainter::fillColor()
{
	return m_state.fillColor;
}

void TextLayoutPainter::setStrokeWidth(double w)
{
	m_state.strokeWidth = w;
}

double TextLayoutPainter::strokeWidth()
{
	return m_state.strokeWidth;
}

void TextLayoutPainter::translate(double x, double y)
{
	m_state.x += x;
	m_state.y += y;
}

double TextLayoutPainter::x()
{
	return m_state.x;
}

double TextLayoutPainter::y()
{
	return m_state.y;
}

void TextLayoutPainter::save()
{
	m_stack.push(m_state);
}

void TextLayoutPainter::restore()
{
	m_state = m_stack.pop();
}

void TextLayoutPainter::scale(double h, double v)
{
	m_state.scaleH = h;
	m_state.scaleV = v;
}

double TextLayoutPainter::getScaleV()
{
	return m_state.scaleV;
}

double TextLayoutPainter::getScaleH()
{
	return m_state.scaleH;
}


TextLayout::TextLayout(StoryText* text, PageItem* frame)
{
	m_story = text;
	m_frame = frame;

	m_validLayout = false;
	m_magicX = 0.0;
	m_lastMagicPos = -1;
	
	m_box = new GroupBox();
}

TextLayout::~TextLayout()
{
	delete m_box;
}


uint TextLayout::lines() const
{
	return m_box->boxes().count();
}

const LineBox* TextLayout::line(uint i) const
{
	return dynamic_cast<const LineBox*>(m_box->boxes()[i]);
}

const Box* TextLayout::box() const
{
	return m_box;
}

Box* TextLayout::box()
{
	return m_box;
}

const PathData& TextLayout::point(int pos) const
{
	return m_path[pos];
}

PathData& TextLayout::point(int pos)
{
	if (pos >= story()->length())
		m_path.resize(story()->length());
	return m_path[pos];
}


void TextLayout::appendLine(LineBox* ls)
{
	assert( ls->firstChar() >= 0 );
	assert( ls->firstChar() < story()->length() );
	assert( ls->lastChar() < story()->length() );
	// HACK: the ascent set by PageItem_TextFrame::layout()
	// is useless, we reset it again based on the y position
	ls->setAscent(ls->y() - m_box->height());
	m_box->addBox(ls);
}

// Remove the last line from the list. Used when we need to backtrack on the layouting.
void TextLayout::removeLastLine ()
{
	m_box->removeBox(m_box->boxes().count() - 1);
}

void TextLayout::render(TextLayoutPainter *p, const StoryText &text)
{
	p->save();
	m_box->render(p, text);
	p->restore();
}

void TextLayout::clear() 
{
	delete m_box;
	m_box = new GroupBox();
	m_path.clear();
	if (m_frame->asPathText() != NULL)
		m_path.resize(story()->length());
}

void TextLayout::setStory(StoryText *story)
{
	m_story = story;
	clear();
}

int TextLayout::startOfLine(int pos) const
{
	for (uint i=0; i < lines(); ++i) {
		const LineBox* ls = line(i);
		if (ls->firstChar() <= pos && pos <= ls->lastChar())
			return ls->firstChar();
	}
	return 0;
}

int TextLayout::endOfLine(int pos) const
{
	for (uint i=0; i < lines(); ++i) {
		const LineBox* ls = line(i);
		if (ls->containsPos(pos))
			return story()->text(ls->lastChar()) == SpecialChars::PARSEP ? ls->lastChar() :
				story()->text(ls->lastChar()) == ' ' ? ls->lastChar() : ls->lastChar() + 1;
	}
	return story()->length();
}

int TextLayout::prevLine(int pos) const
{
	for (uint i=0; i < lines(); ++i)
	{
		// find line for pos
		const LineBox* ls = line(i);
		if (ls->containsPos(pos))
		{
			if (i == 0)
				return startOfLine(pos);
			// find current xpos
			qreal xpos = ls->boundingBox(pos, 1).x();
			if (pos != m_lastMagicPos || xpos > m_magicX)
				m_magicX = xpos;
			
			const LineBox* ls2 = line(i-1);
			// find new cpos
			for (int j = ls2->firstChar(); j <= ls2->lastChar(); ++j)
			{
				xpos = ls2->boundingBox(j,1).x();
				if (xpos > m_magicX) {
					m_lastMagicPos = j;
					return j;
				}
			}
			m_lastMagicPos = ls2->lastChar();
			return ls2->lastChar();
		}
	}
	return m_box->firstChar();
}

int TextLayout::nextLine(int pos) const
{
	for (uint i=0; i < lines(); ++i)
	{
		// find line for pos
		const LineBox* ls = line(i);
		if (ls->containsPos(pos))
		{
			if (i+1 == lines())
				return endOfLine(pos);
			// find current xpos
			qreal xpos = ls->boundingBox(pos, 1).x();

			if (pos != m_lastMagicPos || xpos > m_magicX)
				m_magicX = xpos;
			
			const LineBox* ls2 = line(i+1);
			// find new cpos
			for (int j = ls2->firstChar(); j <= ls2->lastChar(); ++j)
			{
				xpos = ls2->boundingBox(j, 1).x();
				if (xpos > m_magicX) {
					m_lastMagicPos = j;
					return j;
				}
			}
			m_lastMagicPos = ls2->lastChar() + 1;
			return ls2->lastChar() + 1;
		}
	}
	return m_box->lastChar();
}

int TextLayout::startOfFrame() const
{
	return m_box->firstChar();
}

int TextLayout::endOfFrame() const
{
	return m_box->lastChar() + 1;
}


int TextLayout::screenToPosition(FPoint coord) const
{
	return m_box->pointToPosition(coord);
}


FRect TextLayout::boundingBox(int pos, uint len) const
{
	FRect result;
	if (m_box->containsPos(pos))
	{
		result = m_box->boundingBox(pos, len);
		if (result.isValid())
		{
//			result.moveBy(m_frame->xPos(), m_frame->yPos());
			return result;
		}
	}
	
#if 0
	LineBox* ls;
	for (uint i=0; i < lines(); ++i)
	{
		ls = line(i);
		if (ls->lastChar() < pos)
			continue;
		if (ls->firstChar() <= pos) {
			/*
			//if (ls.lastChar == pos && (item(pos)->effects() & ScLayout_SuppressSpace)  )
			{
				if (i+1 < lines())
				{
					ls = line(i+1);
					result.setRect(ls.x, ls.y - ls.ascent, 1, ls.ascent + ls.descent);
				}
				else
				{
					ls = line(lines()-1);
					const ParagraphStyle& pstyle(paragraphStyle(pos));
					result.setRect(ls.x, ls.y + pstyle.lineSpacing() - ls.ascent, 1, ls.ascent + ls.descent);
				}
			}
			else */
			{
				qreal xpos = ls.x;
				for (int j = ls.firstChar; j < pos; ++j)
				{
					if (story()->hasObject(j))
						xpos += (story()->object(j)->width() + story()->object(j)->lineWidth()) * story()->getGlyphs(j)->scaleH;
					else
						xpos += story()->getGlyphs(j)->wide();
				}
				qreal finalw = 1;
				if (story()->hasObject(pos))
					finalw = (story()->object(pos)->width() + story()->object(pos)->lineWidth()) * story()->getGlyphs(pos)->scaleH;
				else
					finalw = story()->getGlyphs(pos)->wide();
				const CharStyle& cs(story()->charStyle(pos));
				qreal desc = -cs.font().descent(cs.fontSize() / 10.0);
				qreal asce = cs.font().ascent(cs.fontSize() / 10.0);
				result.setRect(xpos, ls.y - asce, pos < story()->length()? finalw : 1, desc+asce);
			}
			return result;
		}
	}
	
#endif
	const ParagraphStyle& pstyle(story()->paragraphStyle(qMin(pos, story()->length()))); // rather the trailing style than a segfault.
	if (lines() > 0)
	{
		const LineBox* ls = line(lines()-1);
		result.setRect(ls->x(), ls->y() + pstyle.lineSpacing() - ls->ascent(), 1, ls->height());
	}
	else
	{
		result.setRect(1, 1, 1, pstyle.lineSpacing());
	}

	return result;
}
