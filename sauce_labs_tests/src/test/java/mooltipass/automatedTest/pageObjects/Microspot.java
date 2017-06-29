package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.interactions.Action;
import org.openqa.selenium.interactions.Actions;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Microspot extends AbstractPage{
	
	public Microspot (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(id = "loginForm:emailReg")
	private WebElement email;

	@FindBy(id = "loginForm:passwordReg")
	private WebElement password;

	@FindBy(xpath = "//div[contains(text(),'ANMELDEN')]")
	private WebElement submitLogin;
	
	/*
	@FindBy(xpath = "//a[@href='/msp/pages/registration.jsf']")
	private WebElement loginBtn;
	 */

	@FindBy(xpath = "//span[@class='headerLoginText']")
	private WebElement loginBtn;
	
	@FindBy(xpath = "//form[@id='logoutForm']//a[@href='#']")
	private WebElement logoutBtn;
	



	public void enterEmail(String value){
		waitUntilAppears(email);
		email.sendKeys(value);
	}
	
	public void enterPassword(String value){

		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	public void goToLogin(){
		waitUntilAppears(loginBtn);		
		loginBtn.click();
	}
	
	public void submit(){
	submitLogin.click();
	}
	
	public void logout(){
		waitUntilAppears(logoutBtn);
		logoutBtn.click();
	}
	public boolean checkLogin(){

		waitUntilAppears(By.xpath("//a[contains(text(),'Abmelden')]"));
		return isElementPresent(By.xpath("//a[contains(text(),'Abmelden')]"));
	}
	public boolean checkAtLoginPage(){
		return isElementPresent(By.id("loginForm:emailReg"));
	}
	
}
