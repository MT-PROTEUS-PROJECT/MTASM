#pragma once

namespace GUI {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

    public enum LineNumberStyle { None, OffsetColors, Boxed };

    public ref class LineNumberStrip : Control
    {
    private:
        BufferedGraphics ^_bufferedGraphics;
        BufferedGraphicsContext ^_bufferContext;
        RichTextBox ^_richTextBox;
        Brush ^_fontBrush;
        Brush ^_offsetBrush;
        LineNumberStyle _style;
        Pen ^_penBoxedLine;
        float _fontHeight;
        const float _FONT_MODIFIER;
        bool _hideWhenNoLines, _speedBump;
        const int _DRAWING_OFFSET;

        int _lastYPos, _dragDistance, _lastLineCount;
        int _scrollingLineIncrement, _numPadding;

    public:
        LineNumberStrip(RichTextBox ^plainTextBox) : _bufferContext(BufferedGraphicsManager::Current), _offsetBrush(gcnew SolidBrush(Color::DarkSlateGray)), _FONT_MODIFIER(0.09f), _DRAWING_OFFSET(1), _lastYPos(-1), _scrollingLineIncrement(5), _numPadding(5)
        {
            _richTextBox = plainTextBox;
            plainTextBox->ContentsResized += gcnew System::Windows::Forms::ContentsResizedEventHandler(this, &LineNumberStrip::_richTextBox_ContentsResized);
            plainTextBox->TextChanged += gcnew System::EventHandler(this, &LineNumberStrip::_richTextBox_TextChanged);
            plainTextBox->FontChanged += gcnew System::EventHandler(this, &LineNumberStrip::_richTextBox_FontChanged);
            plainTextBox->VScroll += gcnew System::EventHandler(this, &LineNumberStrip::_richTextBox_VScroll);

            this->SetStyle(ControlStyles::OptimizedDoubleBuffer |
                ControlStyles::AllPaintingInWmPaint | ControlStyles::UserPaint, true);

            this->Size = System::Drawing::Size(10, 10);
            Control::BackColor = System::Drawing::ColorTranslator::FromHtml("#FFFFFF");
            Control::Dock = DockStyle::Left;

            _fontBrush = gcnew SolidBrush(System::Drawing::ColorTranslator::FromHtml("#027aa1"));

            SetFontHeight();
            UpdateBackBuffer();
            this->SendToBack();
        }

    protected:
        void OnMouseDown(MouseEventArgs ^e) override
        {
            Control::OnMouseDown(e);

            if (e->Button.Equals(System::Windows::Forms::MouseButtons::Left) && _scrollingLineIncrement != 0)
            {
                _lastYPos = Cursor->Position.Y;
                this->Cursor = Cursors::NoMoveVert;
            }
        }

        void OnParentChanged(EventArgs ^e) override
        {
            Control::OnParentChanged(e);
            SetControlWidth();
        }

        void OnMouseUp(MouseEventArgs ^e) override
        {
            Control::OnMouseUp(e);
            this->Cursor = Cursors::Default;
        }

        void OnMouseMove(MouseEventArgs ^e) override
        {
            if (e->Button.Equals(System::Windows::Forms::MouseButtons::Left) && _scrollingLineIncrement != 0)
            {
                _dragDistance += Cursor->Position.Y - _lastYPos;

                if (_dragDistance > _fontHeight)
                {
                    int yPos = _richTextBox->ClientSize.Height + (int)(_fontHeight * _scrollingLineIncrement + 0.5f);
                    Point topPos = Point(0, yPos);
                    int index = _richTextBox->GetCharIndexFromPosition(topPos);
                    auto NextLineDown = _richTextBox->GetLineFromCharIndex(index);
                    int selectionStart = _richTextBox->GetFirstCharIndexFromLine(NextLineDown);
                    _richTextBox->Select(selectionStart, 0);
                    _dragDistance = 0;
                }
                else if (_dragDistance < _fontHeight * -1)
                {
                    Point topPos = Point(0, (int)(_fontHeight * (_scrollingLineIncrement * -1) + -0.5f));
                    int index = _richTextBox->GetCharIndexFromPosition(topPos);
                    auto NextLineUp = _richTextBox->GetLineFromCharIndex(index);
                    int selectionStart = _richTextBox->GetFirstCharIndexFromLine(NextLineUp);
                    _richTextBox->Select(selectionStart, 0);
                    _dragDistance = 0;
                }

                _lastYPos = Cursor->Position.Y;
            }
        }

#pragma region Functions
    private:
        void UpdateBackBuffer()
        {
            if (this->Width > 0)
            {
                _bufferContext->MaximumBuffer = System::Drawing::Size(this->Width + 1, this->Height + 1);
                _bufferedGraphics = _bufferContext->Allocate(this->CreateGraphics(), this->ClientRectangle);
            }
        }

        /// <summary>
        /// This method keeps the painted text aligned with the text in the corisponding 
        /// textbox perfectly. GetFirstCharIndexFromLine will yeild -1 if line not
        /// present. GetPositionFromCharIndex will yeild an empty point to char index -1.
        /// To explicitly say that line is not present return -1.
        /// </summary>
        int GetPositionOfRtbLine(int lineNumber)
        {
            int index = _richTextBox->GetFirstCharIndexFromLine(lineNumber);
            Point pos = _richTextBox->GetPositionFromCharIndex(index);
            return index.Equals(-1) ? -1 : pos.Y;
        }

        void SetFontHeight()
        {
            // Shrink the font for minor compensation
            this->Font = gcnew System::Drawing::Font(_richTextBox->Font->FontFamily, (_richTextBox->Font->Size -
                _FONT_MODIFIER) * _richTextBox->ZoomFactor, _richTextBox->Font->Style);

            _fontHeight = _bufferedGraphics->Graphics->MeasureString("123ABC", this->Font).Height;
        }

        void SetControlWidth()
        {
            // Make the line numbers virtually invisble when no lines present
            if (_richTextBox->Lines->Length.Equals(0) && _hideWhenNoLines)
            {
                this->Width = 0;
            }
            else
            {
                int WidthOfWidestLineNumber = 1;
                if (_bufferedGraphics->Graphics != nullptr)
                {
                    System::String ^strNumber = (_richTextBox->Lines->Length).ToString(System::Globalization::CultureInfo::InvariantCulture);
                    SizeF size = _bufferedGraphics->Graphics->MeasureString(strNumber, this->Font);
                    WidthOfWidestLineNumber = (int)(size.Width + 0.5);
                }
                this->Width = WidthOfWidestLineNumber + _numPadding * 2;
            }

            this->Invalidate(false);
        }
#pragma endregion

#pragma region Event Handlers
        System::Void _richTextBox_FontChanged(System::Object ^sender, EventArgs ^e)
        {
            SetFontHeight();
            SetControlWidth();
        }

        System::Void _richTextBox_ContentsResized(System::Object ^sender, ContentsResizedEventArgs ^e)
        {
            SetFontHeight();
            SetControlWidth();
        }

        /// <summary>
        /// Use this event to look for changes in the line count
        /// </summary>
        System::Void _richTextBox_TextChanged(System::Object ^sender, EventArgs ^e)
        {
            // If word wrap is enabled do not check for line changes as new lines
            // from word wrapping will not raise the line changed event

            // Last line count is always equal to current when words are wrapped
            if (_richTextBox->WordWrap || !_lastLineCount.Equals(_richTextBox->Lines->Length))
            {
                SetControlWidth();
            }

            _lastLineCount = _richTextBox->Lines->Length;
        }

    protected:
        void OnForeColorChanged(EventArgs ^e) override
        {
            Control::OnForeColorChanged(e);
            _fontBrush = gcnew SolidBrush(this->ForeColor);
        }

        void OnSizeChanged(EventArgs ^e) override
        {
            Control::OnSizeChanged(e);
            UpdateBackBuffer();
        }

        void OnPaintBackground(PaintEventArgs ^pevent) override
        {
            _bufferedGraphics->Graphics->Clear(this->BackColor);

            int firstIndex = _richTextBox->GetCharIndexFromPosition(Point::Empty);
            int firstLine = _richTextBox->GetLineFromCharIndex(firstIndex);
            Point bottomLeft = Point(0, this->ClientRectangle.Height);
            int lastIndex = _richTextBox->GetCharIndexFromPosition(bottomLeft);
            int lastLine = _richTextBox->GetLineFromCharIndex(lastIndex);

            for (int i = firstLine; i <= lastLine + 1; i++)
            {
                int charYPos = GetPositionOfRtbLine(i);
                if (charYPos.Equals(-1)) continue;
                int yPos = GetPositionOfRtbLine(i) + _DRAWING_OFFSET;

                if (_style == LineNumberStyle::OffsetColors)
                {
                    if (i % 2 == 0)
                    {
                        _bufferedGraphics->Graphics->FillRectangle(_offsetBrush, 0, yPos, this->Width,
                            static_cast<int>(_fontHeight * _FONT_MODIFIER * 10));
                    }
                }
                else if (_style == LineNumberStyle::Boxed)
                {
                    PointF endPos = PointF(static_cast<float>(this->Width), yPos + _fontHeight - _DRAWING_OFFSET * 3);
                    PointF startPos = PointF(0, yPos + _fontHeight - _DRAWING_OFFSET * 3);
                    _bufferedGraphics->Graphics->DrawLine(_penBoxedLine, startPos, endPos);
                }

                PointF stringPos = PointF(static_cast<float>(_numPadding), static_cast<float>(yPos));
                System::String ^line = (i + 1).ToString(System::Globalization::CultureInfo::InvariantCulture);

                _bufferedGraphics->Graphics->DrawString(line, this->Font, _fontBrush, stringPos);

            }

            _bufferedGraphics->Render(pevent->Graphics);
        }

        System::Void _richTextBox_VScroll(System::Object ^sender, EventArgs ^e)
        {
            // Decrease the paint calls by one half when there is more than 3000 lines
            if (_richTextBox->Lines->Length > 3000 && _speedBump)
            {
                _speedBump = !_speedBump;
                return;
            }

            this->Invalidate(false);
        }
#pragma endregion
    };


	/// <summary>
	/// —водка дл€ RTBWithLineNumber
	/// </summary>
	public ref class RTBWithLineNumber : public System::Windows::Forms::UserControl
	{
    private:
        LineNumberStrip ^_strip;

	public:
		RTBWithLineNumber(void)
		{
			InitializeComponent();
            this->_strip = gcnew GUI::LineNumberStrip(this->richTextBox1);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->Controls->Add(_strip);
		}

        System::Windows::Forms::RichTextBox ^get()
        {
            return richTextBox1;
        }

	protected:
		/// <summary>
		/// ќсвободить все используемые ресурсы.
		/// </summary>
		~RTBWithLineNumber()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::RichTextBox ^richTextBox1;
	protected:

	private:
		/// <summary>
		/// ќб€зательна€ переменна€ конструктора.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// “ребуемый метод дл€ поддержки конструктора Ч не измен€йте 
		/// содержимое этого метода с помощью редактора кода.
		/// </summary>
		void InitializeComponent(void)
		{
			this->richTextBox1 = (gcnew System::Windows::Forms::RichTextBox());
			this->SuspendLayout();
			// 
			// richTextBox1
			// 
			this->richTextBox1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->richTextBox1->Location = System::Drawing::Point(0, 0);
			this->richTextBox1->Name = L"richTextBox1";
			this->richTextBox1->Size = System::Drawing::Size(282, 253);
			this->richTextBox1->TabIndex = 0;
			this->richTextBox1->Text = L"";
            this->richTextBox1->ContentsResized += gcnew System::Windows::Forms::ContentsResizedEventHandler(this, &RTBWithLineNumber::richTextBox1_ContentsResized);
			// 
			// RTBWithLineNumber
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(282, 253);
			this->Controls->Add(this->richTextBox1);
			this->Name = L"RTBWithLineNumber";
			this->Text = L"RTBWithLineNumber";
			this->ResumeLayout(false);
            

		}
        System::Void richTextBox1_ContentsResized(System::Object ^sender, ContentsResizedEventArgs ^e)
        {
            if (this->richTextBox1->ZoomFactor > 3.f)
                this->richTextBox1->ZoomFactor = 3.f;
            else if (this->richTextBox1->ZoomFactor < 1.4f)
                this->richTextBox1->ZoomFactor = 1.4f;
        }
#pragma endregion
	};
}
