#pragma once

#include <ASM/ASM.h>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <regex>
#include <bitset>
#include <vector>
#include <msclr/marshal_cppstd.h>

#include "RTBWithLineNumber.h"

namespace GUI
{

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;
    /// <summary>
    /// Сводка для MainForm
    /// </summary>
    public ref class MainForm : public System::Windows::Forms::Form
    {
    private:
        yy::ASM *asm_ = nullptr;
        String ^fileName = System::String::Empty;
        String ^extension = ".mtasm";

    public:
        MainForm(void)
        {
            this->codeBox = gcnew RTBWithLineNumber();

            InitializeComponent();
            //
            //TODO: добавьте код конструктора
            //
        }

    protected:
        /// <summary>
        /// Освободить все используемые ресурсы.
        /// </summary>
        ~MainForm()
        {
            if (components)
            {
                delete components;
            }
        }
    private:
        RTBWithLineNumber ^codeBox;
        System::Windows::Forms::MenuStrip ^menuStrip1;
        System::Windows::Forms::ToolStripMenuItem ^fileToolStripMenuItem;
        System::Windows::Forms::ToolStripMenuItem ^compileToolStripMenuItem;
        System::Windows::Forms::ToolStripMenuItem ^changeViewToolStripMenuItem;
        System::Windows::Forms::TableLayoutPanel ^tableLayoutPanel1;
        System::Windows::Forms::RichTextBox ^infoBox;
        System::Windows::Forms::ToolStripMenuItem ^openToolStripMenuItem;
        System::Windows::Forms::OpenFileDialog ^openFileDialog1;
        System::Windows::Forms::SaveFileDialog ^saveFileDialog1;
        System::Windows::Forms::ToolStripMenuItem ^saveToolStripMenuItem;
        System::Windows::Forms::ToolStripMenuItem ^saveAsToolStripMenuItem;
        System::Windows::Forms::ToolStripMenuItem ^exitToolStripMenuItem;
        bool isTextView;
        System::String ^code;


    private:
        /// <summary>
        /// Обязательная переменная конструктора.
        /// </summary>
        System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
        /// <summary>
        /// Требуемый метод для поддержки конструктора — не изменяйте 
        /// содержимое этого метода с помощью редактора кода.
        /// </summary>
        void InitializeComponent(void)
        {
            this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
            this->fileToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->openToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->saveToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->saveAsToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->exitToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->compileToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->changeViewToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->tableLayoutPanel1 = (gcnew System::Windows::Forms::TableLayoutPanel());
            this->infoBox = (gcnew System::Windows::Forms::RichTextBox());
            this->openFileDialog1 = (gcnew System::Windows::Forms::OpenFileDialog());
            this->saveFileDialog1 = (gcnew System::Windows::Forms::SaveFileDialog());
            this->menuStrip1->SuspendLayout();
            this->tableLayoutPanel1->SuspendLayout();
            this->isTextView = true;
            this->SuspendLayout();
            // 
            // menuStrip1
            // 
            this->menuStrip1->ImageScalingSize = System::Drawing::Size(20, 20);
            this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem ^  >(3)
            {
                this->fileToolStripMenuItem,
                this->compileToolStripMenuItem,
                this->changeViewToolStripMenuItem
            });
            this->menuStrip1->Location = System::Drawing::Point(0, 0);
            this->menuStrip1->Name = L"menuStrip1";
            this->menuStrip1->Size = System::Drawing::Size(1400, 28);
            this->menuStrip1->TabIndex = 1;
            this->menuStrip1->Text = L"menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this->fileToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem ^  >(4)
            {
                this->openToolStripMenuItem,
                    this->saveToolStripMenuItem, this->saveAsToolStripMenuItem, this->exitToolStripMenuItem
            });
            this->fileToolStripMenuItem->Name = L"fileToolStripMenuItem";
            this->fileToolStripMenuItem->Size = System::Drawing::Size(59, 24);
            this->fileToolStripMenuItem->Text = L"Файл";
            // 
            // openToolStripMenuItem
            // 
            this->openToolStripMenuItem->Name = L"openToolStripMenuItem";
            this->openToolStripMenuItem->Size = System::Drawing::Size(192, 26);
            this->openToolStripMenuItem->Text = L"Открыть";
            this->openFileDialog1->Filter = "Ассемблер МТ1804 (*" + extension + ")|*" + extension;
            this->openToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::openToolStripMenuItem_Click);
            // 
            // saveToolStripMenuItem
            // 
            this->saveToolStripMenuItem->Name = L"saveToolStripMenuItem";
            this->saveToolStripMenuItem->Size = System::Drawing::Size(192, 26);
            this->saveToolStripMenuItem->Text = L"Сохранить";
            this->saveToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::saveToolStripMenuItem_Click);
            // 
            // saveAsToolStripMenuItem
            // 
            this->saveAsToolStripMenuItem->Name = L"saveAsToolStripMenuItem";
            this->saveAsToolStripMenuItem->Size = System::Drawing::Size(192, 26);
            this->saveAsToolStripMenuItem->Text = L"Сохранить как";
            this->saveAsToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::saveAsToolStripMenuItem_Click);
            // 
            // exitToolStripMenuItem
            // 
            this->exitToolStripMenuItem->Name = L"exitToolStripMenuItem";
            this->exitToolStripMenuItem->Size = System::Drawing::Size(192, 26);
            this->exitToolStripMenuItem->Text = L"Выход";
            this->exitToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::exitToolStripMenuItem_Click);
            // 
            // compileToolStripMenuItem
            // 
            this->compileToolStripMenuItem->Name = L"compileToolStripMenuItem";
            this->compileToolStripMenuItem->Size = System::Drawing::Size(135, 24);
            this->compileToolStripMenuItem->Text = L"Компилировать";
            this->compileToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::compileToolStripMenuItem_Click);

            this->changeViewToolStripMenuItem->Name = L"changeViewToolStripMenuItem";
            this->changeViewToolStripMenuItem->Size = System::Drawing::Size(135, 24);
            this->changeViewToolStripMenuItem->Text = L"Изменить вид";
            this->changeViewToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::changeViewStripMenuItem_Click);
            // 
            // tableLayoutPanel1
            // 
            this->tableLayoutPanel1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
                | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->tableLayoutPanel1->ColumnCount = 1;
            this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
                100)));
            this->tableLayoutPanel1->Controls->Add(this->codeBox, 0, 0);
            this->tableLayoutPanel1->Controls->Add(this->infoBox, 0, 1);
            this->tableLayoutPanel1->Location = System::Drawing::Point(12, 31);
            this->tableLayoutPanel1->Name = L"tableLayoutPanel1";
            this->tableLayoutPanel1->RowCount = 2;
            this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 85)));
            this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 15)));
            this->tableLayoutPanel1->Size = System::Drawing::Size(1350, 713);
            this->tableLayoutPanel1->TabIndex = 2;
            // 
            // infoBox
            // 
            this->infoBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
                | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->infoBox->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(238)), static_cast<System::Int32>(static_cast<System::Byte>(237)),
                static_cast<System::Int32>(static_cast<System::Byte>(242)));
            this->infoBox->Font = (gcnew System::Drawing::Font(L"Calibri", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(204)));
            this->infoBox->Location = System::Drawing::Point(3, 609);
            this->infoBox->Name = L"infoBox";
            this->infoBox->ReadOnly = true;
            this->infoBox->Size = System::Drawing::Size(1350, 101);
            this->infoBox->TabIndex = 1;
            this->infoBox->Text = L"";
            this->infoBox->ZoomFactor = 2;
            this->infoBox->ContentsResized += gcnew System::Windows::Forms::ContentsResizedEventHandler(this, &MainForm::infoBox_ContentsResized);
            
            this->codeBox->get()->AcceptsTab = true;
            this->codeBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
                | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->codeBox->Font = (gcnew System::Drawing::Font(L"Calibri", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(204)));
            this->codeBox->Location = System::Drawing::Point(4, 4);
            this->codeBox->Margin = System::Windows::Forms::Padding(4);
            this->codeBox->Name = L"codeBox";
            this->codeBox->Size = System::Drawing::Size(1350, 598);
            this->codeBox->TabIndex = 0;
            this->codeBox->Text = L"";
            
            // 
            // MainForm
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(255)));
            this->ClientSize = System::Drawing::Size(1400, 753);
            this->Controls->Add(this->tableLayoutPanel1);
            this->Controls->Add(this->menuStrip1);
            this->MainMenuStrip = this->menuStrip1;
            this->Name = L"MainForm";
            this->Text = L"MTASM";
            this->menuStrip1->ResumeLayout(false);
            this->menuStrip1->PerformLayout();
            this->tableLayoutPanel1->ResumeLayout(false);
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion
    private: System::Void changeViewStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e)
    {
        if (isTextView)
        {
            try
            {
                std::istringstream in(this->codeBox->getFullCode());
                if (!in)
                {
                    MessageBox::Show(this, "Не удалось прочитать код программы. Попробуйте снова.", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);
                    return;
                }

                std::stringstream out;

                this->asm_ = new (std::nothrow) yy::ASM(in, out);
                if (!this->asm_)
                {
                    MessageBox::Show(this, "Не удалось аллоцировать память для компилятора. Попробуйте снова.", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);
                    return;
                }

                auto compileRes = this->asm_->Parse();
                auto compilerErrors = this->asm_->GetEC().Get(ExceptionContainer::Tag::ICE);
                auto unexpectedErrors = this->asm_->GetEC().Get(ExceptionContainer::Tag::OTHER);
                auto syntaxErrors = this->asm_->GetEC().Get(ExceptionContainer::Tag::SE);
                auto cmds = this->asm_->GetCmds();
                this->codeBox->highlightCmds(cmds);

                delete this->asm_;
                this->asm_ = nullptr;

                if (unexpectedErrors.empty() && compilerErrors.empty() && syntaxErrors.empty())
                {
                    code = this->codeBox->get()->Text;
                    isTextView = false;
                    bool first = true;
                    std::string total_code = "|_ADDRESS__|____AR_____|_CA_ |_M1|I8-I6_|_M0|I2-I0_|_C0|I5-I3_|__A__|__B__|____D___ |\n";
                    std::string line;
                    static std::regex reg(R"(([0-9]+)\t([0-9]+)\t([0-9]+)[\r]*)");
                    std::smatch match;
                    while (std::getline(out, line))
                    {
                        if (!std::regex_match(line, match, reg))
                        {
                            if (!first)
                                total_code += '\n';
                            total_code += line;
                        }
                        else
                        {
                            total_code += '\n';
                            std::bitset<10> addr_bits(std::stoul(match[1].str()));
                            total_code += "| " + addr_bits.to_string();
                            total_code += " | ";
                            addr_bits = std::stoul(match[2].str());
                            total_code += addr_bits.to_string() + " | ";
                            
                            uint32_t mtemuFmt = std::stoul(match[3].str());
                            uint32_t mask = 0xF0000000;
                            std::bitset<4> bits_4;
                            for (size_t i = 0; i < 6; ++i)
                            {
                                bits_4 = ((mtemuFmt & mask) >> 4 * (7 - i));
                                
                                if (i >= 1 && i <= 2)
                                {
                                    total_code += "     " + bits_4.to_string() + "      | ";
                                }
                                else if (i == 3)
                                {
                                    total_code += "     " + bits_4.to_string() + "     | ";
                                }
                                else
                                {
                                    total_code += bits_4.to_string() + " | ";
                                }
                                mask >>= 4;
                            }
                            
                            std::bitset<8> bits_8(mtemuFmt & 0xFF);
                            total_code += bits_8.to_string() + " |";
                        }
                        first = false;
                    }
                    this->codeBox->get()->Text = msclr::interop::marshal_as<System::String^>(total_code);
                    this->codeBox->get()->ReadOnly = true;
                }
                else
                {
                    this->infoBox->ResetText();
                    LogToInfoBox("Не удалось перевести в двоичный вид т.к. код содержит ошибки");
                }
            }
            catch (Exception^ ex)
            {
                if (this->asm_)
                {
                    delete this->asm_;
                    this->asm_ = nullptr;
                }
                MessageBox::Show(this, ex->Message, "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
        }
        else
        {
            this->codeBox->get()->Text = code;
            this->codeBox->get()->ReadOnly = false;
            this->codeBox->highlighCmdsPos();
            isTextView = true;
        }
    }

    private: System::Void compileToolStripMenuItem_Click(System::Object ^sender, System::EventArgs ^e)
    {
        try
        {
            std::stringstream in;
            if (!isTextView)
            {
                in << msclr::interop::marshal_as<std::string>(System::String::Copy(this->code));
            }
            else
            {
                in << this->codeBox->getFullCode();
            }

            System::String ^path;
            if (fileName == System::String::Empty)
            {
                path = System::IO::Directory::GetCurrentDirectory();
            }
            else
            {
                path = System::IO::Path::GetDirectoryName(fileName);
            }
            path += "\\out.bin";

            std::ofstream out(msclr::interop::marshal_as<std::string>(path), std::ios::out | std::ios::binary);
            if (!out)
            {
                MessageBox::Show(this, "Не удалось открыть файл для записи скомпилированного кода. Попробуйте снова.", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);
                return;
            }

            this->asm_ = new (std::nothrow) yy::ASM(in, out);
            if (!this->asm_)
            {
                MessageBox::Show(this, "Не удалось аллоцировать память для компилятора. Попробуйте снова.", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);
                return;
            }

            this->infoBox->ResetText();
            LogToInfoBox("Компиляция запущена.");

            auto compileRes = this->asm_->Parse();
            auto compilerErrors = this->asm_->GetEC().Get(ExceptionContainer::Tag::ICE);
            auto unexpectedErrors = this->asm_->GetEC().Get(ExceptionContainer::Tag::OTHER);
            auto syntaxErrors = this->asm_->GetEC().Get(ExceptionContainer::Tag::SE);
            auto cmds = this->asm_->GetCmds();
            this->codeBox->highlightCmds(cmds);

            delete this->asm_;
            this->asm_ = nullptr;
            out.close();

            if (!unexpectedErrors.empty())
            {
                std::string totalErr;
                for (const auto &err : unexpectedErrors)
                    totalErr += err._msg + '\n';
                System::String ^msg = gcnew System::String(("Критические ошибки компиляции:\n" + totalErr + "\nПопробуйте снова.").c_str());
                MessageBox::Show(this, msg, "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);
                return;
            }

            if (!compilerErrors.empty())
            {
                std::string totalErr;
                for (const auto &err : compilerErrors)
                    totalErr += err._msg + '\n';
                System::String ^msg = gcnew System::String(("Ошибки компиляции:\n" + totalErr + "\nПопробуйте снова.").c_str());
                MessageBox::Show(this, msg, "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);
                return;
            }

            LogToInfoBox("Компиляция завершена с кодом " + (compileRes + syntaxErrors.size()).ToString() + ".");
            if (syntaxErrors.empty())
            {
                LogToInfoBox("Ошибок: 0.");
                LogToInfoBox("Создан файл " + path + ". Размер: " + System::IO::FileInfo(path).Length.ToString() + " байт.");
            }
            else
            {
                LogToInfoBox("Ошибки:");
                for (const auto &err : syntaxErrors)
                    LogToInfoBox("Стр. " + err._loc.value_or(yy::location()).begin.line.ToString() + ". " + gcnew String(err._msg.c_str()));
            }
        }
        catch (Exception ^ex)
        {
            if (this->asm_)
            {
                delete this->asm_;
                this->asm_ = nullptr;
            }
            MessageBox::Show(this, ex->Message, "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

    private: System::Void openToolStripMenuItem_Click(System::Object ^sender, System::EventArgs ^e)
    {
        try
        {
            if (this->openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK)
            {
                this->codeBox->get()->LoadFile(this->openFileDialog1->FileName, RichTextBoxStreamType::PlainText);
                fileName = this->openFileDialog1->FileName;
                this->openFileDialog1->FileName = System::String::Empty;
            }
        }
        catch (Exception ^ex)
        {
            MessageBox::Show(this, ex->Message, "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

    private: System::Void saveToolStripMenuItem_Click(System::Object ^sender, System::EventArgs ^e)
    {
        try
        {
            if (fileName == System::String::Empty)
            {
                this->saveFileDialog1->FileName = "untitled.mtasm";
                if (this->saveFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK)
                {
                    this->codeBox->get()->SaveFile(this->saveFileDialog1->FileName, RichTextBoxStreamType::PlainText);
                    fileName = this->saveFileDialog1->FileName;
                }
            }
            else
            {
                this->codeBox->get()->SaveFile(fileName, RichTextBoxStreamType::PlainText);
            }
            LogToInfoBox("Файл: " + fileName + " сохранен");
        }
        catch (Exception ^ex)
        {
            MessageBox::Show(this, ex->Message, "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

    private: System::Void saveAsToolStripMenuItem_Click(System::Object ^sender, System::EventArgs ^e)
    {
        try
        {
            this->saveFileDialog1->FileName = System::IO::Path::GetFileName(fileName);
            if (this->saveFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK)
            {
                this->codeBox->get()->SaveFile(this->saveFileDialog1->FileName, RichTextBoxStreamType::PlainText);
                fileName = this->saveFileDialog1->FileName;
            }
        }
        catch (Exception ^ex)
        {
            MessageBox::Show(this, ex->Message, "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

    private: System::Void LogToInfoBox(System::String ^msg)
    {
        this->infoBox->Text += DateTime::Now.ToString("HH:mm:ss") + " " + msg + "\n";
    }

    private: System::Void infoBox_ContentsResized(System::Object ^sender, ContentsResizedEventArgs ^e)
    {
        if (this->infoBox->ZoomFactor > 3.f)
            this->infoBox->ZoomFactor = 3.f;
        else if (this->infoBox->ZoomFactor < 1.4f)
            this->infoBox->ZoomFactor = 1.4f;
    }

    private: System::Void exitToolStripMenuItem_Click(System::Object ^sender, System::EventArgs ^e)
    {
        try
        {
            this->Close();
        }
        catch (Exception ^ex)
        {
            MessageBox::Show(this, ex->Message, "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }
};
}
