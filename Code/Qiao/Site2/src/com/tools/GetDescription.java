package com.tools;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.util.Vector;

/*
 * 使用方法，iport该包
 * GetDescription gd=new GetGescription();
 * Vector<String> v=gd.getDescription(5);
 * 
 * 对应id请见/Leafdata/leafdata33.sql
 *
 */
public class GetDescription {
//	public static void main(String[] args) {
//		Vector<String> v=getDescription(2);
//		for(int i=0;i<v.size();i++){
//			System.out.println(v.get(i));
//		}
//	}
	
	public  Vector<String> getDescription(int leafId){
		//Vector<String> ret;
		Connection ct=null;
		PreparedStatement ps=null;
		try {
			Class.forName("com.mysql.jdbc.Driver");	
			ct=DriverManager.getConnection("jdbc:mysql://localhost:3306/leaf","root","6356763");
			ps=ct.prepareStatement("select * from leaf where id=?");
			ps.setInt(1, leafId);
			ResultSet rs=ps.executeQuery();
			if(rs.next())
			{
				//System.out.println("aa");
				String ds=rs.getString("description");
				//System.out.println(ds);
				return parse(ds);
			}else{
				System.out.println("Wrong input");
			}
		
		} catch (Exception e) {
			// TODO: handle exception
			e.printStackTrace();
		}
		return null;
		
	}
	
	public static Vector<String> parse(String s){
		Vector<String> ret=new Vector<String>();
		while(s.length()>0){
			//System.out.println(s.length());
			String sub="";
			for(int i=0;i<s.length();i++){
				if(s.charAt(i)==' ' || s.charAt(i)=='\n'){
					continue;
				}
				else if(s.charAt(i)!='。'){
					sub+=s.charAt(i);
				}else{
					//System.out.println(i);
					sub+=s.charAt(i);
					//System.out.println(sub);
					//int slen=s.length();
					s=s.substring(i+1);
					//System.out.println(s);
					ret.addElement(sub);
					//System.out.println("add");
					break;
				}
			}
		}
		System.out.println(ret.size());
		return ret;
		
	}
}
