using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace TemplateApp
{
    public partial class DxControl : UserControl
    {
        private Timer renderTimer;
        private Stopwatch stopwatch;
        private double previousElapsedSeconds;
        bool initialized=false;
        bool enablePaint;
        public bool EnablePaint
        {
            get { return enablePaint; }
            set { 
                enablePaint = value;  
                if(enablePaint)
                {
                    if (!initialized)
                    {
                        DllImportFunctions.InitDirectX((int)Handle);
                        DllImportFunctions.PrepareScene((int)Handle, Width, Height);
                        initialized = true;
                    }
                }
                }
        }
        public DxControl()
        {
            InitializeComponent();
            InitializeRenderTimer();
            InitializeStopwatch();
        }
        private void InitializeRenderTimer()
        {
            renderTimer = new Timer();
            renderTimer.Interval = 16;
            renderTimer.Tick += (sender, e) => Invalidate();
            renderTimer.Start();
        }

        private void InitializeStopwatch()
        {
            stopwatch = Stopwatch.StartNew();
            previousElapsedSeconds = 0;
        }
        protected override void OnPaintBackground(PaintEventArgs pevent) { }

        private void DxControl_Paint(object sender, PaintEventArgs e)
        {
            if (initialized)
            {
                double currentElapsedSeconds = stopwatch.ElapsedMilliseconds / 1000d;
                double deltaSeconds = currentElapsedSeconds - previousElapsedSeconds;
                previousElapsedSeconds = currentElapsedSeconds;

                DllImportFunctions.RenderScene((int)Handle, deltaSeconds);
            }
        }

        private void DxControl_SizeChanged(object sender, EventArgs e)
        {
            if (initialized)
            {
                EnablePaint = true;
            }

            Invalidate();
        }        
    }
}
public class DllImportFunctions
{
    [DllImport("DirectXCppCode.Dll", CallingConvention = CallingConvention.Cdecl, EntryPoint = "InitDirectX")]
    public static extern void InitDirectX(int hwnd);

    [DllImport("DirectXCppCode.Dll", CallingConvention = CallingConvention.Cdecl, EntryPoint = "PrepareScene")]
    public static extern void PrepareScene(int hwnd, int w, int h);

    [DllImport("DirectXCppCode.Dll", CallingConvention = CallingConvention.Cdecl, EntryPoint = "RenderScene")]
    public static extern void RenderScene(int hwnd, double deltaTime);
}
