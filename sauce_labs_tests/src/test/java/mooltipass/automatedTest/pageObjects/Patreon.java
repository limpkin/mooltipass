package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Patreon extends AbstractPage{
	
	public Patreon (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(xpath = "//a[contains(text(),'Log In')]")
	private WebElement loginBtn;
	@FindBy(xpath = "//input[@name='email']")
	private WebElement email;

	@FindBy(xpath = "//input[@name='password']")
	private WebElement password;
	
	@FindBy(xpath = "//div[contains(text(),'Log in')]")
	private WebElement submitLogin;
	
	
	@FindBy(xpath = "//a[contains(@href,'logout')]")
	private WebElement logoutBtn;
									
	@FindBy(xpath = "//div[@data-reactid='.0.0.1.2.1.1:2.0.0']")
	private WebElement dashBoard;
	
	public void enterEmail(String value){
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	public void goToLogin(){
		loginBtn.click();
	}
	
	public void submit(){

		waitUntilAppears(submitLogin);
		submitLogin.click();
	}
	public void goTodDashboard()
	{
		hover(dashBoard);	
	}
	public void logout(){
		logoutBtn.click();
	}
	
	public boolean checkLogin(){
		waitUntilAppears(By.xpath( "//div[@class='_3Dg-components-Avatar--xs _3ba-styles-shared--roundImage _2kC-components-Avatar--avatarBackground']"));
		return isElementPresent(By.xpath( "//div[@class='_3Dg-components-Avatar--xs _3ba-styles-shared--roundImage _2kC-components-Avatar--avatarBackground']"));
	}
	public boolean checkAtLoginPage(){
		waitUntilAppears(By.xpath("//input[@name='email']"));
		return isElementPresent(By.xpath("//input[@name='email']"));
	}

}
