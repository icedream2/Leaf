import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.List;
import java.util.Scanner;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.tomcat.util.http.fileupload.DiskFileUpload;
import org.apache.tomcat.util.http.fileupload.FileItem;
import org.apache.tomcat.util.http.fileupload.FileUploadException;



public class PhonegapServlet extends HttpServlet {
	private static final long serialVersionUID = 1L;
	private static long id = 0;
	
	protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		response.setContentType("text/html");
		System.out.println("doing get...");
		String callbackFuncName = (String)request.getParameter("callback");
		String x = (String)request.getParameter("data");
		
		System.out.println("x= "+x);
		//System.out.println("y= "+y);
		//Socket client = new Socket("192.168.168.129", 54577);
		StringBuffer sb = new StringBuffer();
		String tmp=null;
		tmp="x= "+x;
		sb.append("({");
		sb.append("val:");
		sb.append("'");
		sb.append(tmp);
		sb.append("'");
		sb.append("})");
		
		Socket client = new Socket("192.168.168.129", 54577);
		OutputStream client_out = client.getOutputStream();
		InputStream in = new FileInputStream("D:/tmp.jpg");
		client_out.write('!');
		byte[] buffer = new byte[1024];
		while (in.read(buffer) != -1) {
			client_out.write(buffer);
		}
		
		final byte[] delim = "\r\n\r\n".getBytes();
		client_out.write(delim);
		client_out.write(x.getBytes());
		client_out.write(delim);
		
		in.close();
		
		Scanner scan = new Scanner(client.getInputStream());
		String s = scan.nextLine();
		System.out.println("Recieve from server: " + s);
		
		client.close();
		
		PrintWriter out = response.getWriter();
		out.write(callbackFuncName+sb.toString());
	}


	protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		response.setContentType("text/html;charset=UTF-8");
//		PrintWriter out = response.getWriter();
		System.out.println("doing post.........");
		DiskFileUpload disFileUpload = new DiskFileUpload();
		try{
			@SuppressWarnings("unchecked")
			List<FileItem> list = disFileUpload.parseRequest(request);
			for(FileItem fileItem:list){
				if(fileItem.isFormField()){
					
				}else{
					if("fileAddPic".equals(fileItem.getFieldName())){
						
						
						InputStream ins = fileItem.getInputStream();
						
					//	Socket client = new Socket("192.168.168.129", 54577);
					//	OutputStream out = client.getOutputStream();
						/// @TODO Modify the file name
						OutputStream ofs = new FileOutputStream("D:/tmp.jpg");
						
						byte[] bufferd = new byte[1024];
					//	out.write('!');
						while (ins.read(bufferd) != -1) {
						//	out.write(bufferd);
							ofs.write(bufferd);
						}
					//	out.write("\r\n\r\n".getBytes());
					
						ofs.close();
						
						//Scanner scan = new Scanner(client.getInputStream());
						//String s = scan.nextLine();
						//System.out.println("Recieve from server: " +	 s);
						
						
						
						PrintWriter pw=response.getWriter();
						pw.write("success");
						//pw.write(s);
						//scan.close();
						//client.close();
						System.out.println("Transfered Image");
					}
				}
			}
		}catch(FileUploadException e){
			
		}
	}

}