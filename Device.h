#pragma once

using namespace System;

namespace Dagnabnet {
	/// <summary>
	/// 
	/// </summary>
	public ref class Device {
	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="buffer">The buffer.</param>
		delegate void ReceiveHandler(array<Byte>^ buffer);
	public:
		/// <summary>
		/// Initializes a new instance of the <see cref="Device" /> class.
		/// </summary>
		Device();
	public:		
		/// <summary>
		/// Opens the specified name.
		/// </summary>
		/// <param name="name">The name.</param>
		void Open(System::String^ name);
		
		/// <summary>
		/// Configures this instance.
		/// </summary>
		void Configure();
		
		/// <summary>
		/// Configures the specified parameters.
		/// </summary>
		/// <param name="parameters">The parameters.</param>
		void Configure(System::String^ parameters);

		/// <summary>
		/// Resets this instance.
		/// </summary>
		void Reset();
		
		/// <summary>
		/// Attaches this instance.
		/// </summary>
		void Attach();
		
		/// <summary>
		/// Attaches the specified stream.
		/// </summary>
		/// <param name="stream">The stream.</param>
		/// <param name="flags">The flags.</param>
		/// <param name="extra">The extra.</param>
		void Attach(int stream, int flags, int extra);
		
		/// <summary>
		/// Starts this instance.
		/// </summary>
		void Start();
		
		/// <summary>
		/// Closes this instance.
		/// </summary>
		void Close();
	private:
		/// <summary>
		/// Polls this instance.
		/// </summary>
		void Poll();
	private:	
		ReceiveHandler^ _receive;
		System::Threading::CancellationTokenSource _cts;
		int _fd;
		int _stream;
	public:		
		event ReceiveHandler^ Receive
		{
		public:
			void add(ReceiveHandler^ value)
			{
				_receive += value;
			}
			void remove(ReceiveHandler^ value)
			{
				_receive -= value;
			}
		protected:
			void raise(array<Byte>^ buffer)
			{
				auto handle = _receive;
				if(handle)
				{
					handle->Invoke(buffer);
				}
			}
		}
	};

}